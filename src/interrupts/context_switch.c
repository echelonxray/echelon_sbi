#include "context_switch.h"
#include <globals.h>
#include <main.h>
#include <entry.h>
#include <string.h>
#include <thread_locking.h>
#include <sbi_commands.h>
#include <printm.h>
#include <inc/types.h>
#include <inc/cpu.h>
#include <inc/reg.h>
#include <inc/csr.h>
#include <inc/memmap.h>
#include <drivers/uart.h>

extern ksemaphore_t* hart_command_que_locks;
extern ksemaphore_t* sbi_hsm_locks;
extern volatile sint32_t* sbi_hsm_states;
extern volatile CPU_Context* hart_contexts_user;
extern volatile Hart_Command* hart_commands;
extern uintRL_t kernel_load_to_point;

extern __thread uintRL_t mhartid;
extern __thread uintRL_t hart_has_menvcfg;

void print_reg_state(volatile CPU_Context* cpu_context) {
	printm("----Reg State----\n");
	printm("\tHart ID: %u\n", mhartid);
	printm("\tExecution Mode: %u\n", cpu_context->execution_mode);
	printm("\tSATP: %08lX\n", (unsigned long)CSRI_BITCLR(CSR_SATP, 0));
	printm("\tSTVEC: %08lX\n", (unsigned long)CSRI_BITCLR(CSR_STVEC, 0));
	printm("\t PC: %08lX\n", cpu_context->regs[REG_PC]); // REG_PC == 0
	
	size_t i = 1;
	while (i <= 8) {  //  x1 through x8
		printm("\t x%u: %08lX\t x%u: %08lX\n", i, cpu_context->regs[i], i + 1, cpu_context->regs[i + 1]);
		i += 2;
	}
	{                 //  x9 through x10
		printm("\t x%u: %08lX\tx%u: %08lX\n", i, cpu_context->regs[i], i + 1, cpu_context->regs[i + 1]);
		i += 2;
	}
	while (i <= 30) { // x11 through x30
		printm("\tx%u: %08lX\tx%u: %08lX\n", i, cpu_context->regs[i], i + 1, cpu_context->regs[i + 1]);
		i += 2;
	}
	{                 // x31
		printm("\tx%u: %08lX\n", i, cpu_context->regs[i]);
	}
	return;
}

void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value, uintRL_t mtval) {
	if        (cause_value == 3) {
		// M-Mode Software Interrupt -- This is a hart command
		
		// Save the command locally so that it does not get clobbered when the msip flag is cleared.
		Hart_Command command = hart_commands[mhartid];
		
		volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
		
		if        (command.command == HARTCMD_SWITCHCONTEXT) {
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			switch_context((CPU_Context*)(command.param0));
		} else if (command.command == HARTCMD_STARTHART) {
			printm("\tStarting: %u\n", mhartid);
			
// QEMU will refuse to execute below M-Mode if the PMP registers aren't configured.
#ifdef MM_QEMU_VIRT
#if   __riscv_xlen == 128
			#error "Not Implemented"
#elif __riscv_xlen == 64
			CSRI_WRITE(CSR_PMPADDR0, (0x0000000080000000ul >> 2) & 0x003FFFFFFFFFFFFFul);
			CSRI_WRITE(CSR_PMPADDR1, (kernel_load_to_point >> 2) & 0x003FFFFFFFFFFFFFul);
			CSRI_WRITE(CSR_PMPADDR2, (0xFFFFFFFFFFFFFFFFul >> 2) & 0x003FFFFFFFFFFFFFul);
			CSRI_WRITE(CSR_PMPCFG0, (0x0F << 16) | (0x08 <<  8) | (0x0B <<  0));
#elif __riscv_xlen == 32
			CSRI_WRITE(CSR_PMPADDR0, (0x80000000ul         >> 2) & 0xFFFFFFFFul);
			CSRI_WRITE(CSR_PMPADDR1, (kernel_load_to_point >> 2) & 0xFFFFFFFFul);
			CSRI_WRITE(CSR_PMPADDR2, (0xFFFFFFFFul         >> 2) & 0xFFFFFFFFul);
			CSRI_WRITE(CSR_PMPCFG0, (0x0F << 16) | (0x08 <<  8) | (0x0B <<  0));
#endif
			// QEMU Fixes - sstc (stimecmp)
			//   Background:
			//   In commit 43888c2f1823212b1064a6a94d65d8acaf954478 QEMU added RISC-V sstc extension support.
			//   This extension introduces the stimecmp CSR to allow the Supervisor (Kernel) to directly
			//   program the timer interrupt rather than relying on SBI calls.  This introduction also added
			//   sstc to the ISA extension list.  It seems that the Linux Kernel is probing for sstc support.
			//   If detected, it will attempt to use and set the stimecmp (and stimecmph if rv32) registers.
			//   However, the sstc extension gates access to the stimecmp (and stimecmph if rv32) registers
			//   behind bitflags in MENVCFG and MCOUNTEREN.  If these are not set by firmware, supervisor
			//   attempts to access stimecmp (or stimecmph if rv32) will throw an Illegal Instruction
			//   Exception.
			CSRI_BITSET(CSR_MCOUNTEREN, 0x2);
			if (hart_has_menvcfg) {
#if   __riscv_xlen == 128
				#error "Not Implemented"
#elif __riscv_xlen == 64
				CSRI_BITSET(CSR_MENVCFG, 1 << 63);
#elif __riscv_xlen == 32
				CSRI_BITSET(CSR_MENVCFGH, 1 << 31);
#endif
			}

			// QEMU Fixes - Zicboz, Zicbom, Zicbop
			//   Background:
			//   In commit a939c500793ae7672defe5e3dc83220576a7b202 QEMU added RISC-V Zicboz extension support.
			//   In commit e05da09b7cfd8dd08c55e77ab2106634f7b06ad9 QEMU added RISC-V Zicbom extension support.
			//   In commit 59cb29d6a5149871d1acb18fb465879b1af5f3b2 QEMU added RISC-V Zicbop extension support.
			//   Then in commit 007698632814b4b4aeae1a9c176d932951e9c8cf QEMU added property descriptions for the
			//   Zicbom operations to the Flattened Device Tree binary image.  It seems that the Linux Kernel uses
			//   the presence of these properties in the FDT as an indication that these functions are available
			//   on the host platform.  However, while QEMU did introduce support, the specification for these
			//   extensions gates their availability to lower privilege modes behind CSR bitflags in xENVCFG.
			//   If firmware doesn't turn these bitflags on, when the Kernel attempts to use them, an Illegal
			//   Instruction Exception from S-mode will be generated.  An alternaive fix is to build the Kernel
			//   without Zicbom support.
			if (hart_has_menvcfg) {
				CSRI_BITSET(CSR_MENVCFG, 3 << 4);
				CSRI_BITSET(CSR_MENVCFG, 1 << 6);
				CSRI_BITSET(CSR_MENVCFG, 1 << 7);
			}
#endif

			// Do not attempt to delegate Exceptions (Not supported in custom emulator)
			CSRI_WRITE(CSR_MEDELEG, 0x0000);
			// Delegate S-Mode Software Interrupt to S-Mode
			// Delegate S-Mode Timer Interrupt to S-Mode
			// Delegate S-Mode External Interrupt to S-Mode
			CSRI_WRITE(CSR_MIDELEG, 0x0222);
			
			// Enable S-Mode Timer Interrupt
			// Enable S-Mode Software Interrupt
			// Enable M-Mode Software Interrupt
			CSRI_WRITE( CSR_MIE, 0x02A);
			// Clear all pending Interrupts
			CSRI_BITCLR(CSR_MIP, 0xFFF);
			
			// Clear Virtual Address CSR
			CSRI_WRITE(CSR_SATP, 0);
			// Disable M-Mode Interrupts
			// Disable S-Mode Interrupts
			// Disable S-Mode Previous Interrupt Enable
			CSRI_BITCLR(CSR_MSTATUS, 0x2A);
			// Enable M-Mode Previous Interrupt Enable
			CSRI_BITSET(CSR_MSTATUS, 0x80);
			
			clear_hart_context(hart_contexts_user + mhartid);
			hart_contexts_user[mhartid].context_id = mhartid;
			hart_contexts_user[mhartid].execution_mode = EM_S;
			hart_contexts_user[mhartid].regs[REG_PC] = command.param1;
			hart_contexts_user[mhartid].regs[REG_A0] = mhartid;
			hart_contexts_user[mhartid].regs[REG_A1] = command.param2;
			
			ksem_wait(sbi_hsm_locks + mhartid);
			sbi_hsm_states[mhartid] = SBI_HSM_STARTED;
			ksem_post(sbi_hsm_locks + mhartid);
			
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			switch_context(hart_contexts_user + mhartid);
		} else if (command.command == HARTCMD_REMOTE_FENCE_I) {
			CPU_FENCEI();
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA) {
			// TODO: Specific parameters
			CPU_SFENCEVMA();
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA_ASID) {
			// TODO: Specific parameters
			CPU_SFENCEVMA();
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SMODE_SOFTINT) {
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			
			// Trigger an S-Mode software interrupt
			CSRI_BITSET(CSR_MIP, 0x2);
		} else {
			goto not_handled_interrupt;
		}
	} else if (cause_value == 1) {
		// S-Mode Software Interrupt
		
		printm("ESBI Error: S-Mode Software Interrupt trapped into M-Mode.  Halting.\n");
		idle_loop();
	} else if (cause_value == 5) {
		// S-Mode Timer Interrupt
		
		printm("ESBI Error: S-Mode Timer Interrupt trapped into M-Mode.  Halting.\n");
		idle_loop();
	} else if (cause_value == 9) {
		// S-Mode External Interrupt
		
		printm("ESBI Error: S-Mode External Interrupt trapped into M-Mode.  Halting.\n");
		idle_loop();
	} else if (cause_value == 7) {
		// M-Mode Timer Interrupt
		
		// Bounce this into S-Mode
		CSRI_BITCLR(CSR_MIE, 0x80);
		CSRI_BITSET(CSR_MIP, 0x20);
	} else if (cause_value == 11) {
		// M-Mode External Interrupt
		
		printm("ESBI Error: M-Mode External Int received in M-Mode.  Halting.\n");
		CSRI_BITCLR(CSR_MIE, 0x800);
		idle_loop();
	} else {
		not_handled_interrupt:
		printm("ESBI Interrupt!  Lower 31 mcause bits: %lu.  Halting.\n", cause_value);
		idle_loop();
	}
	
	return;
}

void exception_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value, uintRL_t mtval) {
	//printm("ESBI: EXCEPTION!!!!\n");
	//print_reg_state(cpu_context);
	//idle_loop();

	if        (cause_value == 0) {
		// Instruction Address Misaligned
		//CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		//printm("Instruction Address Misaligned\n");
		//print_reg_state(cpu_context);
		//idle_loop();
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 1) {
		// Instruction Access Fault
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("Instruction Access Fault\n");
		print_reg_state(cpu_context);
		idle_loop();
		//s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 2) {
		// Illegal Instruction
		//printm("Illegal Instruction\n");
		//print_reg_state(cpu_context);
		//idle_loop();
		
		uintRL_t csr_satp;
		csr_satp = CSRI_BITCLR(CSR_SATP, 0);
		uint32_t* instruction = (void*)walk_pts(cpu_context->regs[REG_PC], csr_satp, 0);
		
		dec_inst dinst;
		uintRL_t form = decode_instruction(*instruction, &dinst);
		//uintRL_t form = decode_instruction(mtval, &dinst);
		if (form) {
			
			// Emulate reading the CSR: time (or timeh on rv32)
			if (dinst.opcode == 0x73) {
				if (dinst.funct3 == 0x2 || dinst.funct3 == 0x3 || dinst.funct3 == 0x6 || dinst.funct3 == 0x7) {
					if (dinst.rs1 == 0) {
#if defined(MM_QEMU_VIRT) && 0
#if   __riscv_xlen == 128
						#error "Not Implemented"
#elif __riscv_xlen == 64
						if        (dinst.imm == 0xC01) {
							// CSR 64: time
							uint64_t* mtime = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIME);
							if (dinst.rd != 0) {
								cpu_context->regs[dinst.rd] = *mtime;
							}
							cpu_context->regs[REG_PC] += 4;
							return;
						}
#elif __riscv_xlen == 32
						if        (dinst.imm == 0xC01) {
							// CSR 32: time
							uint32_t* mtime = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIME);
							uint32_t mtime_val = *mtime;
							if (dinst.rd != 0) {
								cpu_context->regs[dinst.rd] = mtime_val;
							}
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						if        (dinst.imm == 0xC81) {
							// CSR 32: timeh
							uint64_t* mtime = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIME + 4);
							uint32_t mtime_val = *mtime;
							if (dinst.rd != 0) {
								cpu_context->regs[dinst.rd] = mtime_val;
							}
							cpu_context->regs[REG_PC] += 4;
							return;
						}
#endif
#endif
					}
				}
			}
		}
		
		if (cpu_context->execution_mode == EM_U) {
			s_delegation_trampoline(cpu_context, cause_value, mtval);
		}
		
		printm("Illegal Instruction: 0x%08X\n", *instruction);
		print_reg_state(cpu_context);
		idle_loop();
	} else if (cause_value == 3) {
		// Breakpoint
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("Breakpoint Exception.  Trap Handler: M-Mode\n");
		print_reg_state(cpu_context);
		idle_loop();
		//return;
	} else if (cause_value == 4) {
		// Load Address Misaligned
		//CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		//printm("Load Address Misaligned\n");
		//print_reg_state(cpu_context);
		//idle_loop();
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 5) {
		// Load Access Fault
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("Load Access Fault\n");
		print_reg_state(cpu_context);
		idle_loop();
		//s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 6) {
		// Store/AMO Address Misaligned
		//CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		//printm("Store/AMO Address Misaligned\n");
		//print_reg_state(cpu_context);
		//idle_loop();
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 7) {
		// Store/AMO Access Fault
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("Store/AMO Access Fault\n");
		print_reg_state(cpu_context);
		idle_loop();
		//s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 8) {
		// User-Mode Environment Exception
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 9) {
		// Supervisor-Mode Environment Exception
		cpu_context->regs[REG_PC] += 4;

		sintRL_t params[6];
		params[0] = cpu_context->regs[REG_A0];
		params[1] = cpu_context->regs[REG_A1];
		params[2] = cpu_context->regs[REG_A2];
		params[3] = cpu_context->regs[REG_A3];
		params[4] = cpu_context->regs[REG_A4];
		params[5] = cpu_context->regs[REG_A5];

		struct sbiret sbi_return;
		sbi_return = call_to_sbi(cpu_context->regs[REG_A7], cpu_context->regs[REG_A6], params);
		cpu_context->regs[REG_A1] = sbi_return.value;
		cpu_context->regs[REG_A0] = sbi_return.error;
		return;
	} else if (cause_value == 11) {
		// Machine-Mode Environment Exception
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("ESBI Trap Caught!  Machine-Mode Environment Exception.  Trap Handler: M-Mode. mhartid: %lu.  Halting.\n", mhartid);
		print_reg_state(cpu_context);
		idle_loop();
	} else if (cause_value == 12) {
		// Instruction Page Fault
		//printm("Instruction Page Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 13) {
		// Load Page Fault
		//printm("Load Page Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 15) {
		// Store/AMO Page Fault
		//printm("Store/AMO Page Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else {
		CSRI_BITCLR(CSR_MSTATUS, 0x8); // Disable Interrupts.
		printm("ESBI Exception!  Lower 31 mcause bits: %lu.  Halting.\n", cause_value);
		print_reg_state(cpu_context);
		idle_loop();
	}
	
	return;
}

uintRL_t decode_instruction(uint32_t einst, dec_inst* dinst) {
	uint32_t opcode = einst & 0x7F;
	
	einst >>= 7;
	union {
		struct encoded_type_r enc_r;
		struct encoded_type_i enc_i;
		struct encoded_type_s enc_s;
		struct encoded_type_b enc_b;
		struct encoded_type_u enc_u;
		struct encoded_type_j enc_j;
	} params;
	memcpy(&params, &einst, sizeof(uint32_t));
	
	if        (opcode == 0x73) {
		// OpCode: SYSTEM, Encoding: I-Type
		
		dinst->opcode = opcode;
		dinst->rd = params.enc_i.rd;
		dinst->funct3 = params.enc_i.funct3;
		dinst->rs1 = params.enc_i.rs1;
		dinst->imm = params.enc_i.imm;
		
		dinst->rs2 = 0;
		dinst->funct7 = 0;
		return 2;
	} else if (opcode == 0x33) {
		// OpCode: Mul32, Encoding: R-Type
		
		dinst->opcode = opcode;
		dinst->rd = params.enc_r.rd;
		dinst->funct3 = params.enc_r.funct3;
		dinst->rs1 = params.enc_r.rs1;
		dinst->rs2 = params.enc_r.rs2;
		dinst->funct7 = params.enc_r.funct7;
		
		dinst->imm = 0;
		return 1;
	}
	
	return 0;
}

// This function assumes the PT structure is valid.  Do not call it if that is not guaranteed.
// This function assumes the PT structures are not changed (Such as by other cores).
// This function makes no security checks.  It assumes PTs structure is safe.  A malicious
// kernel could exploit it.
uintRL_t walk_pts(uintRL_t location, uintRL_t csr_satp, uintRL_t debug) {
	uintRL_t mem_addr;
	
#if   __riscv_xlen == 128
	// 128-Bit XLEN
	
	#error "Virtual Memory parsing is not implemented for 128-bit XLEN"
	
#elif __riscv_xlen == 64
	// 64-Bit XLEN
	
	sint64_t page_walk = (sint64_t)csr_satp;
	// Is Virtual Memory Active?
	if ((csr_satp >> 60) == 8) { // Sv39
		// Shift to match PTE entry offset to ready for entry to the PT Walk loop
		page_walk <<= 10;
		
		// Walk the PTs
		uintRL_t shift_ammount = 9 + 9 + 12;
		do {
			// Correct Offset: Left Shift and then Right Arithmetic Shift for Sign Extension
			page_walk <<= 10;
			page_walk >>= 8;
			
			sint64_t* page_ptr = (sint64_t*)(page_walk & ~((uintRL_t)0xFFF));
			
			if (debug) {
				printm("Page Table Address: ");
				uint32_t hibits = (((uint64_t)page_ptr) >> 32) & 0xFFFFFFFF;
				uint32_t lobits = (((uint64_t)page_ptr) >>  0) & 0xFFFFFFFF;
				char buf[20];
				itoa(hibits, buf, 20, -16, 8);
				printm(buf);
				printm("_");
				itoa(lobits, buf, 20, -16, 8);
				printm(buf);
				printm("\n");
			}
			
			page_walk = page_ptr[(location >> shift_ammount) & 0x1FF];
			
			if (debug) {
				printm("Entry In The Page Table: ");
				uint32_t hibits = (((uint64_t)page_walk) >> 32) & 0xFFFFFFFF;
				uint32_t lobits = (((uint64_t)page_walk) >>  0) & 0xFFFFFFFF;
				char buf[20];
				itoa(hibits, buf, 20, -16, 8);
				printm(buf);
				printm("_");
				itoa(lobits, buf, 20, -16, 8);
				printm(buf);
				printm("\n");
			}
			
			shift_ammount -= 9;
		} while ((page_walk & 0xF) == 1 && shift_ammount >= 12);
		
		// Correct Offset: Left Shift and then Right Arithmetic Shift for Sign Extension
		// Clear the Lower 12 Bits: Over Right Shift by 12 and then Shift back Left by 12 Bits
		page_walk <<= 10;
		page_walk >>= 20;
		page_walk <<= 12;
		
		// Is this a Superpage?
		// Append the additional lower bits if so.
		// Note: A Superpage is only valid per the RISC-V Privileged Specification if properly 
		//       aligned.  If an omitted PPN is non-zero, the Superpage is considered misaligned.
		//       This will cause the Hardware to raise a Page-Fault Exception.  Therefore, we 
		//       do not need to clear them.  We can assume they are already set to 
		//       zero in the page_walk variable and just Binary OR against it.
		while (shift_ammount >= 12) {
			page_walk |= (sint64_t)(location & (0x1FF << shift_ammount));
			shift_ammount -= 9;
		}
		
		// Binary OR the last 12 bits of the address to the computed physical memory page.
		page_walk |= location & 0xFFF;
		mem_addr = (uintRL_t)page_walk;
		
		if (debug) {
			printm("Final Address: ");
			uint32_t hibits = (mem_addr >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (mem_addr >>  0) & 0xFFFFFFFF;
			char buf[20];
			itoa(hibits, buf, 20, -16, 8);
			printm(buf);
			printm("_");
			itoa(lobits, buf, 20, -16, 8);
			printm(buf);
			printm("\n");
		}
		
	} else {
		if (debug) {
			printm("No Page Walk\n");
		}
		mem_addr = (uintRL_t)location;
	}
	
#else
	// 32-Bit XLEN
	
	// Get the inital value of the satp CSR
	sint32_t page_walk = (sint32_t)csr_satp;
	
	// Is Virtual Memory Active?
	if ((csr_satp >> 31) == 1) {
		// Shift to match PTE entry offset to ready for entry to the PT Walk loop
		page_walk <<= 10;
		
		// Walk the PTs
		uintRL_t shift_ammount = 10 + 12;
		do {
			// Correct Offset: Left Shift and then Right Arithmetic Shift for Sign Extension
			page_walk <<= 2;
			page_walk >>= 0;
			
			sint32_t* page_ptr = (sint32_t*)(page_walk & ~((uintRL_t)0xFFF));
			page_walk = page_ptr[(location >> shift_ammount) & 0x3FF];
			shift_ammount -= 10;
		} while ((page_walk & 0xF) == 1 && shift_ammount >= 12);
		
		// Clear the Lower 12 Bits
		// Correct Offset
		page_walk >>= 10;
		page_walk <<= 12;
		
		// Is this a Superpage?
		// Append the additional lower bits if so.
		// Note: A Superpage is only valid per the RISC-V Privileged Specification if properly 
		//       aligned.  If an omitted PPN is non-zero, the Superpage is considered misaligned.
		//       This will cause the Hardware to raise a Page-Fault Exception.  Therefore, we 
		//       do not need to clear them.  We can assume they are already set to 
		//       zero in the page_walk variable and just Binary OR against it.
		while (shift_ammount >= 12) {
			page_walk |= (sint32_t)(location & (0x3FF << shift_ammount));
			shift_ammount -= 10;
		}
		
		// Binary OR the last 12 bits of the address to the computed physical memory page.
		page_walk |= location & 0xFFF;
		mem_addr = (uintRL_t)page_walk;
	} else {
		mem_addr = (uintRL_t)location;
	}
	
#endif
	
	return mem_addr;
}

void clear_hart_context(volatile CPU_Context* hart_context) {
	hart_context->context_id = 0;
	hart_context->execution_mode = 0;
	hart_context->reserved_0 = 0;
	for (uintRL_t j = 0; j < 32; j++) {
		hart_context->regs[j] = 0;
	}
	return;
}

void send_hart_command_que(uintRL_t hart_id, Hart_Command* command) {
	CPU_SFENCEVMA();
	CPU_FENCEI();
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	ksem_post(hart_command_que_locks + hart_id);
	return;
}

void send_hart_command_lck(uintRL_t hart_id, Hart_Command* command) {
	CPU_SFENCEVMA();
	CPU_FENCEI();
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	return;
}

void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command) {
	CPU_SFENCEVMA();
	CPU_FENCEI();
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	ksem_post(hart_command_que_locks + hart_id);
	return;
}

void send_hart_command_ret(uintRL_t hart_id, Hart_Command* command) {
	CPU_SFENCEVMA();
	CPU_FENCEI();
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	*command = hart_commands[hart_id];
	ksem_post(hart_command_que_locks + hart_id);
	return;
}
