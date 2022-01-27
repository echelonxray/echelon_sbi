#include "./context_switch.h"
#include "./../../inc/types.h"
#include "./../../inc/string.h"
#include "./../../inc/kernel_calls.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../kernel.h"
#include "./../thread_locking.h"
#include "./../sbi_commands.h"
#include "./../kstart_entry.h"
#include "./../drivers/uart.h"
#include "./../globals.h"
#include "./../debug.h"

extern ksemaphore_t* hart_command_que_locks;
extern ksemaphore_t* sbi_hsm_locks;
extern volatile sint32_t* sbi_hsm_states;
extern volatile CPU_Context* hart_contexts_user;
extern volatile Hart_Command* hart_commands;
extern uintRL_t kernel_load_to_point;

extern __thread uintRL_t mhartid;

void print_reg_state(volatile CPU_Context* cpu_context) {
	char buf[20];
	
	DEBUG_print("----Reg State----\n");
	DEBUG_print("\tHart ID: ");
	itoa(mhartid, buf, 20, -10, 0);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tExecution Mode: ");
	itoa(cpu_context->execution_mode, buf, 20, -10, 0);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	uintRL_t satp;
	DEBUG_print("\tSATP: ");
	__asm__ __volatile__ ("csrr %0, satp" : "=r" (satp));
	itoa(satp, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	uintRL_t stvec;
	DEBUG_print("\tSTVEC: ");
	__asm__ __volatile__ ("csrr %0, stvec" : "=r" (stvec));
	itoa(stvec, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	DEBUG_print("\t PC: ");
	itoa(cpu_context->regs[REG_PC], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	/*
	uint32_t* inst = (uint32_t*)(cpu_context->regs[REG_PC]);
	DEBUG_print("\tINST: 0x");
	itoa(*inst, buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	*/
	
	DEBUG_print("\t x1: ");
	itoa(cpu_context->regs[REG_X1], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\t x2: ");
	itoa(cpu_context->regs[REG_X2], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\t x3: ");
	itoa(cpu_context->regs[REG_X3], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\t x4: ");
	itoa(cpu_context->regs[REG_X4], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\t x5: ");
	itoa(cpu_context->regs[REG_X5], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\t x6: ");
	itoa(cpu_context->regs[REG_X6], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\t x7: ");
	itoa(cpu_context->regs[REG_X7], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\t x8: ");
	itoa(cpu_context->regs[REG_X8], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\t x9: ");
	itoa(cpu_context->regs[REG_X9], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx10: ");
	itoa(cpu_context->regs[REG_X10], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx11: ");
	itoa(cpu_context->regs[REG_X11], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx12: ");
	itoa(cpu_context->regs[REG_X12], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx13: ");
	itoa(cpu_context->regs[REG_X13], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx14: ");
	itoa(cpu_context->regs[REG_X14], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx15: ");
	itoa(cpu_context->regs[REG_X15], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx16: ");
	itoa(cpu_context->regs[REG_X16], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx17: ");
	itoa(cpu_context->regs[REG_X17], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx18: ");
	itoa(cpu_context->regs[REG_X18], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx19: ");
	itoa(cpu_context->regs[REG_X19], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx20: ");
	itoa(cpu_context->regs[REG_X20], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx21: ");
	itoa(cpu_context->regs[REG_X21], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx22: ");
	itoa(cpu_context->regs[REG_X22], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx23: ");
	itoa(cpu_context->regs[REG_X23], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx24: ");
	itoa(cpu_context->regs[REG_X24], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx25: ");
	itoa(cpu_context->regs[REG_X25], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx26: ");
	itoa(cpu_context->regs[REG_X26], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx27: ");
	itoa(cpu_context->regs[REG_X27], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx28: ");
	itoa(cpu_context->regs[REG_X28], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx29: ");
	itoa(cpu_context->regs[REG_X29], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\tx30: ");
	itoa(cpu_context->regs[REG_X30], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\tx31: ");
	itoa(cpu_context->regs[REG_X31], buf, 20, -16, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	return;
}

void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value, uintRL_t mtval) {
	char str[30];
	//DEBUG_print("interrupt_c_handler\n");
	/*
	if (cpu_context->execution_mode == 3) {
		DEBUG_print("<<<<<<<<<<<<<<<<<<<<interrupt_c_handler: ");
		itoa(mhartid, str, 30, 10, 0);
		DEBUG_print(str);
		DEBUG_print(" ");
		itoa(cause_value, str, 30, 10, 0);
		DEBUG_print(str);
		DEBUG_print(" 0x");
		uintRL_t tmp_reg;
		__asm__ __volatile__ ("csrr %0, mie" : "=r" (tmp_reg));
		itoa(tmp_reg, str, 30, -16, -3);
		DEBUG_print(str);
		DEBUG_print("\n");
		//idle_loop();
	}
	*/
	
	if        (cause_value == 3) {
		// M-Mode Software Interrupt -- This is a hart command
		
		// Save the command locally so that it does not get clobbered when the
		// msip flag is cleared.
		Hart_Command command = hart_commands[mhartid];
		
		volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
		
		if        (command.command == HARTCMD_SWITCHCONTEXT) {
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			switch_context((CPU_Context*)(command.param0));
		} else if (command.command == HARTCMD_STARTHART) {
			DEBUG_print("\tStarted: ");
			itoa(mhartid, str, 30, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			//__asm__ __volatile__ ("csrs mie, %0" : : "r" (1 << 7));
			
#ifdef MM_QEMU_VIRT

#if   __riscv_xlen == 64
			__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" ((0x0000000080000000ul >> 2) & 0x003FFFFFFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpaddr1, %0" : : "r" ((kernel_load_to_point >> 2) & 0x003FFFFFFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpaddr2, %0" : : "r" ((0xFFFFFFFFFFFFFFFFul >> 2) & 0x003FFFFFFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" ((0x0F << 16) | (0x08 <<  8) | (0x0B <<  0)));
#elif __riscv_xlen == 32
			__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" ((0x80000000ul         >> 2) & 0xFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpaddr1, %0" : : "r" ((kernel_load_to_point >> 2) & 0xFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpaddr2, %0" : : "r" ((0xFFFFFFFFul         >> 2) & 0xFFFFFFFFul));
			__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" ((0x0F << 16) | (0x08 <<  8) | (0x0B <<  0)));
#endif

#endif
			
			__asm__ __volatile__ ("csrw medeleg, %0" : : "r" (0x0000));
			__asm__ __volatile__ ("csrw mideleg, %0" : : "r" (0x0222));
			
			__asm__ __volatile__ ("csrw mie, %0"     : : "r" (0x02A));
			__asm__ __volatile__ ("csrc mip, %0"     : : "r" (0xFFF));
			
			__asm__ __volatile__ ("csrw satp, zero");
			__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x2A));
			__asm__ __volatile__ ("csrs mstatus, %0" : : "r" (0x80));
			
			clear_hart_context(hart_contexts_user + mhartid);
			hart_contexts_user[mhartid].context_id = mhartid;
			hart_contexts_user[mhartid].execution_mode = EM_S;
			hart_contexts_user[mhartid].regs[REG_PC] = command.param1;
			//hart_contexts_user[mhartid].regs[REG_A0] = command.param0;
			hart_contexts_user[mhartid].regs[REG_A0] = mhartid;
			hart_contexts_user[mhartid].regs[REG_A1] = command.param2;
			
			//ksem_wait(sbi_hsm_locks + mhartid);
			sbi_hsm_states[mhartid] = SBI_HSM_STARTED;
			//ksem_post(sbi_hsm_locks + mhartid);
			
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			//DEBUG_print("Started\n");
			switch_context(hart_contexts_user + mhartid);
		} else if (command.command == HARTCMD_REMOTE_FENCE_I) {
			__asm__ __volatile__ ("fence.i");
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA) {
			// TODO: Specific parameters
			__asm__ __volatile__ ("sfence.vma zero, zero");
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA_ASID) {
			// TODO: Specific parameters
			__asm__ __volatile__ ("sfence.vma zero, zero");
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SMODE_SOFTINT) {
			DEBUG_print("M-Mode Soft Interrupt\n");
			
			hart_commands[mhartid].command = 0;
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x2));
		} else {
			goto not_handled_interrupt;
		}
	} else if (cause_value == 1) {
		// S-Mode Timer Interrupt
		
		DEBUG_print("S-Mode Soft Interrupt\n");
		idle_loop();
	} else if (cause_value == 5) {
		// S-Mode Timer Interrupt
		
		DEBUG_print("S-Mode Timer Interrupt\n");
		idle_loop();
	} else if (cause_value == 9) {
		// S-Mode External Interrupt
		
		DEBUG_print("S-Mode External Interrupt\n");
		idle_loop();
	} else if (cause_value == 7) {
		// M-Mode Timer Interrupt
		
		//DEBUG_print("M-Mode Timer Interrupt\n");
		__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x80));
		__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x20));
	} else if (cause_value == 11) {
		// M-Mode External Interrupt
		
		DEBUG_print("M-Mode External Int received in M-Mode\n");
		__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x800));
		//__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x200));
	} else {
		not_handled_interrupt:
		DEBUG_print("ESBI Interrupt!  Lower mcause bits: ");
		itoa(cause_value, str, 30, 10, 0);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
	}
	
	return;
}

void exception_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value, uintRL_t mtval) {
	/*
	char buf[20];
	DEBUG_print("exception_c_handler: ");
	itoa(cause_value, buf, 20, 10, 0);
	DEBUG_print(buf);
	DEBUG_print("\n");
	print_reg_state(cpu_context);
	*/
	
	/*
	if (cause_value != 2 && cause_value != 9) {
		char buf[20];
		DEBUG_print("exception_c_handler: ");
		itoa(cause_value, buf, 20, 10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		print_reg_state(cpu_context);
	}
	*/
	
	if        (cause_value == 0) {
		// Instruction Address Misaligned
		DEBUG_print("Instruction Address Misaligned\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 1) {
		// Instruction Access Fault
		DEBUG_print("Instruction Access Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 2) {
		// Illegal Instruction
		//DEBUG_print("Illegal Instruction\n");
		
		uintRL_t csr_satp;
		__asm__ __volatile__ ("csrr %0, satp" : "=r" (csr_satp));
		uint32_t* instruction = (void*)walk_pts(cpu_context->regs[REG_PC], csr_satp, 0);
		
		dec_inst dinst;
		uintRL_t form = decode_instruction(*instruction, &dinst);
		//uintRL_t form = decode_instruction(mtval, &dinst);
		//DEBUG_print("Trace X\n");
		if (form) {
			//DEBUG_print("Trace Z\n");
			
			// Emulate reading the CSR: time(timeh)
			if (dinst.opcode == 0x73) {
				if (dinst.funct3 == 0x2 || dinst.funct3 == 0x3 || dinst.funct3 == 0x6 || dinst.funct3 == 0x7) {
					if (dinst.rs1 == 0) {
#ifdef MM_QEMU_VIRT
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
							// CSR 32: time
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
			
#ifdef MM_CUSTOM_EMU
#if   __riscv_xlen == 32
			// Emulate the M-Extension (But only for the Supervisor)
			if (dinst.opcode == 0x33 && cpu_context->execution_mode == 1) {
				if (dinst.funct7 == 0x1) {
					if (dinst.rd == 0) {
						cpu_context->regs[REG_PC] += 4;
						return;
					}
					if        (dinst.funct3 == 0x0) {
						/*
						{
							DEBUG_print("M-Ext Emulate: MUL\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// MUL
						sintRL_t rs1 = cpu_context->regs[dinst.rs1];
						sintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						cpu_context->regs[dinst.rd] = rs1 * rs2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x1) {
						/*
						{
							DEBUG_print("M-Ext Emulate: MULH\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// MULH
						uintRL_t rs1 = cpu_context->regs[dinst.rs1];
						uintRL_t rs2 = cpu_context->regs[dinst.rs2];
						sintRL_t rb1 = rs1;
						sintRL_t rb2 = rs2;
						
						if (rb1 < 0) {
							rs1 = ~rs1 + 1;
						}
						if (rb2 < 0) {
							rs2 = ~rs2 + 1;
						}
						
						uint32_t inverse_shift = sizeof(sintRL_t) * 8;
						uintRL_t lo_bits = 0;
						uintRL_t hi_bits = 0;
						for (uint32_t i = 0; i < sizeof(sintRL_t) * 8; i++) {
							if (rs2 & 0x1) {
								// Upon Entry to the loop, inverse_shift will overflow the 
								// shift.  In RISC-V shifts by a register are only by the 
								// lower 5, 6, or 7 bits in the register for XLENs 32, 64, 128 
								// respectively.  Shift overflows are undefined behaviour 
								// in the C standard.  However, RISC-V will not do anything 
								// because the lowest bits will be zero.
								if (inverse_shift < sizeof(sintRL_t) * 8) {
									hi_bits += rs1 >> inverse_shift;
								}
								
								uintRL_t tmpval = lo_bits;
								tmpval += rs1 << i;
								if (tmpval < lo_bits) {
									// Overflow
									hi_bits++;
								}
								lo_bits = tmpval;
							}
							rs2 >>= 1;
							inverse_shift--;
						}
						
						rb1 ^= rb2;
						rb2 = hi_bits;
						if (rb1 < 0) {
							rb2 = ~rb2;
							lo_bits = ~lo_bits;
							if (lo_bits == (uintRL_t)-1) {
								rb2++;
							}
						}
						
						cpu_context->regs[dinst.rd] = rb2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x2) {
						/*
						{
							DEBUG_print("M-Ext Emulate: MULHSU\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// MULHSU
						uintRL_t rs1 = cpu_context->regs[dinst.rs1];
						uintRL_t rs2 = cpu_context->regs[dinst.rs2];
						sintRL_t rb1 = rs1;
						sintRL_t rb2 = rs2;
						
						if (rb1 < 0) {
							rs1 = ~rs1 + 1;
						}
						
						uint32_t inverse_shift = sizeof(sintRL_t) * 8;
						uintRL_t lo_bits = 0;
						uintRL_t hi_bits = 0;
						for (uint32_t i = 0; i < sizeof(sintRL_t) * 8; i++) {
							if (rs2 & 0x1) {
								// Upon Entry to the loop, inverse_shift will overflow the 
								// shift.  In RISC-V shifts by a register are only by the 
								// lower 5, 6, or 7 bits in the register for XLENs 32, 64, 128 
								// respectively.  Shift overflows are undefined behaviour 
								// in the C standard.  I have it wrapped them in IF statements.
								if (inverse_shift < sizeof(sintRL_t) * 8) {
									hi_bits += rs1 >> inverse_shift;
								}
								
								uintRL_t tmpval = lo_bits;
								tmpval += rs1 << i;
								if (tmpval < lo_bits) {
									// Overflow
									hi_bits++;
								}
								lo_bits = tmpval;
							}
							rs2 >>= 1;
							inverse_shift--;
						}
						
						rb2 = hi_bits;
						if (rb1 < 0) {
							rb2 = ~rb2;
							lo_bits = ~lo_bits;
							if (lo_bits == (uintRL_t)-1) {
								rb2++;
							}
						}
						
						cpu_context->regs[dinst.rd] = rb2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x3) {
						/*
						{
							DEBUG_print("M-Ext Emulate: MULHU\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// MULHU
						uintRL_t rs1 = cpu_context->regs[dinst.rs1];
						uintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						uint32_t inverse_shift = sizeof(sintRL_t) * 8;
						uintRL_t lo_bits = 0;
						uintRL_t hi_bits = 0;
						for (uint32_t i = 0; i < sizeof(sintRL_t) * 8; i++) {
							if (rs2 & 0x1) {
								// Upon Entry to the loop, inverse_shift will overflow the 
								// shift.  In RISC-V shifts by a register are only by the 
								// lower 5, 6, or 7 bits in the register for XLENs 32, 64, 128 
								// respectively.  Shift overflows are undefined behaviour 
								// in the C standard.  However, RISC-V will not do anything 
								// because the lowest bits will be zero.
								if (inverse_shift < sizeof(sintRL_t) * 8) {
									hi_bits += rs1 >> inverse_shift;
								}
								
								uintRL_t tmpval = lo_bits;
								tmpval += rs1 << i;
								if (tmpval < lo_bits) {
									// Overflow
									hi_bits++;
								}
								lo_bits = tmpval;
							}
							rs2 >>= 1;
							inverse_shift--;
						}
						
						cpu_context->regs[dinst.rd] = hi_bits;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x4) {
						/*
						{
							DEBUG_print("M-Ext Emulate: DIV\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// DIV
						sintRL_t rs1 = cpu_context->regs[dinst.rs1];
						sintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						if (rs2 == 0) {
							cpu_context->regs[dinst.rd] = (sintRL_t)-1;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						if (rs2 == ((sintRL_t)-1) && rs1 == (((sintRL_t)1) << (sizeof(sintRL_t) * 8 - 1))) {
							cpu_context->regs[dinst.rd] = rs1;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						
						cpu_context->regs[dinst.rd] = rs1 / rs2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x5) {
						/*
						{
							DEBUG_print("M-Ext Emulate: DIVU\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// DIVU
						uintRL_t rs1 = cpu_context->regs[dinst.rs1];
						uintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						if (rs2 == 0) {
							cpu_context->regs[dinst.rd] = (sintRL_t)-1;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						
						cpu_context->regs[dinst.rd] = rs1 / rs2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else if (dinst.funct3 == 0x6) {
						/*
						{
							DEBUG_print("M-Ext Emulate: REM\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// REM
						sintRL_t rs1 = cpu_context->regs[dinst.rs1];
						sintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						if (rs2 == 0) {
							cpu_context->regs[dinst.rd] = rs1;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						if (rs2 == ((sintRL_t)-1) && rs1 == (((sintRL_t)1) << (sizeof(sintRL_t) * 8 - 1))) {
							cpu_context->regs[dinst.rd] = 0;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						
						cpu_context->regs[dinst.rd] = rs1 % rs2;
						cpu_context->regs[REG_PC] += 4;
						return;
					} else {//(dinst.funct7 == 0x7)
						/*
						{
							DEBUG_print("M-Ext Emulate: REMU\n");
							print_reg_state(cpu_context);
						}
						*/
						
						// REMU
						uintRL_t rs1 = cpu_context->regs[dinst.rs1];
						uintRL_t rs2 = cpu_context->regs[dinst.rs2];
						
						if (rs2 == 0) {
							cpu_context->regs[dinst.rd] = rs1;
							cpu_context->regs[REG_PC] += 4;
							return;
						}
						
						cpu_context->regs[dinst.rd] = rs1 % rs2;
						cpu_context->regs[REG_PC] += 4;
						return;
					}
				}
			}
#endif
#endif
		}
		
		//DEBUG_print("Illegal Instruction\n");
		
		{
			DEBUG_print("Illegal Instruction: ");
			char buf[20];
			itoa(*instruction, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
			print_reg_state(cpu_context);
			idle_loop();
		}
		
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 3) {
		// Breakpoint
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		DEBUG_print("ESBI Trap Caught!  Breakpoint Exception.  Trap Handler: M-Mode\n");
		cpu_context->regs[REG_PC] += 4;
		//for (volatile uintRL_t i = 0; i < 200000000; i++) {}
		idle_loop();
		return;
	} else if (cause_value == 4) {
		// Load Address Misaligned
		DEBUG_print("Load Address Misaligned\n");
		//print_reg_state(cpu_context);
		//idle_loop();
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 5) {
		// Load Access Fault
		DEBUG_print("Load Access Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 6) {
		// Store/AMO Address Misaligned
		DEBUG_print("Store/AMO Address Misaligned\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 7) {
		// Store/AMO Access Fault
		DEBUG_print("Store/AMO Access Fault\n");
		print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 8) {
		// User-Mode Environment Exception
		//cpu_context->regs[REG_PC] += 4;
		DEBUG_print("User-Mode Environment Exception\n");
		//print_reg_state(cpu_context);
		//__asm__ ("ebreak");
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 9) {
		// Supervisor-Mode Environment Exception
		//DEBUG_print("Supervisor-Mode Environment Exception\n");
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
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		DEBUG_print("ESBI Trap Caught!  Machine-Mode Environment Exception.  Trap Handler: M-Mode. mhartid: \n");
		char buf[20];
		itoa(mhartid, buf, 20, -10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		idle_loop();
		//cpu_context->regs[REG_PC] += 4;
	} else if (cause_value == 12) {
		// Instruction Page Fault
		//DEBUG_print("Instruction Page Fault\n");
		
		/*
		char buf[20];
		uintRL_t satp;
		uintRL_t tmpv;
		
		{
			DEBUG_print("satp: ");
			__asm__ __volatile__ ("csrr %0, satp" : "=r" (satp));
			uint32_t hibits = (satp >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (satp >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			DEBUG_print("misa: ");
			__asm__ __volatile__ ("csrr %0, misa" : "=r" (tmpv));
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			DEBUG_print("mstatus: ");
			__asm__ __volatile__ ("csrr %0, mstatus" : "=r" (tmpv));
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			tmpv = walk_pts(cpu_context->regs[REG_A0], satp, 1);
			DEBUG_print("Walked: ");
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		*/
		
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 13) {
		// Load Page Fault
		//DEBUG_print("Load Page Fault\n");
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else if (cause_value == 15) {
		// Store/AMO Page Fault
		//DEBUG_print("Store/AMO Page Fault\n");
		
		/*
		char buf[20];
		uintRL_t satp;
		uintRL_t tmpv;
		
		{
			DEBUG_print("satp: ");
			__asm__ __volatile__ ("csrr %0, satp" : "=r" (satp));
			uint32_t hibits = (satp >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (satp >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			DEBUG_print("misa: ");
			__asm__ __volatile__ ("csrr %0, misa" : "=r" (tmpv));
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			DEBUG_print("mstatus: ");
			__asm__ __volatile__ ("csrr %0, mstatus" : "=r" (tmpv));
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
		{
			tmpv = walk_pts(cpu_context->regs[REG_A0], satp, 1);
			DEBUG_print("Walked: ");
			uint32_t hibits = (tmpv >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (tmpv >>  0) & 0xFFFFFFFF;
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		*/
		
		//print_reg_state(cpu_context);
		s_delegation_trampoline(cpu_context, cause_value, mtval);
	} else {
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		char buf[20];
		DEBUG_print("ESBI Exception!  Lower mcause bits: ");
		itoa(cause_value, buf, 20, 10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
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
				DEBUG_print("Page Table Address: ");
				uint32_t hibits = (((uint64_t)page_ptr) >> 32) & 0xFFFFFFFF;
				uint32_t lobits = (((uint64_t)page_ptr) >>  0) & 0xFFFFFFFF;
				char buf[20];
				itoa(hibits, buf, 20, -16, 8);
				DEBUG_print(buf);
				DEBUG_print("_");
				itoa(lobits, buf, 20, -16, 8);
				DEBUG_print(buf);
				DEBUG_print("\n");
			}
			
			page_walk = page_ptr[(location >> shift_ammount) & 0x1FF];
			
			if (debug) {
				DEBUG_print("Entry In The Page Table: ");
				uint32_t hibits = (((uint64_t)page_walk) >> 32) & 0xFFFFFFFF;
				uint32_t lobits = (((uint64_t)page_walk) >>  0) & 0xFFFFFFFF;
				char buf[20];
				itoa(hibits, buf, 20, -16, 8);
				DEBUG_print(buf);
				DEBUG_print("_");
				itoa(lobits, buf, 20, -16, 8);
				DEBUG_print(buf);
				DEBUG_print("\n");
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
			DEBUG_print("Final Address: ");
			uint32_t hibits = (mem_addr >> 32) & 0xFFFFFFFF;
			uint32_t lobits = (mem_addr >>  0) & 0xFFFFFFFF;
			char buf[20];
			itoa(hibits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("_");
			itoa(lobits, buf, 20, -16, 8);
			DEBUG_print(buf);
			DEBUG_print("\n");
		}
		
	} else {
		if (debug) {
			DEBUG_print("No Page Walk\n");
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
	uint32_t foo = 0;
	__asm__ __volatile__ ("sfence.vma");
	__asm__ __volatile__ ("fence.i");
	__asm__ __volatile__ ("amoor.w.aqrl zero, zero, (%0)" : : "r" (&foo));
	//DEBUG_print("SND CMD0\n");
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	ksem_post(hart_command_que_locks + hart_id);
	return;
}

void send_hart_command_lck(uintRL_t hart_id, Hart_Command* command) {
	uint32_t foo = 0;
	__asm__ __volatile__ ("sfence.vma");
	__asm__ __volatile__ ("fence.i");
	__asm__ __volatile__ ("amoor.w.aqrl zero, zero, (%0)" : : "r" (&foo));
	DEBUG_print("SND CMD1\n");
	
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	return;
}

void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command) {
	uint32_t foo = 0;
	__asm__ __volatile__ ("sfence.vma");
	__asm__ __volatile__ ("fence.i");
	__asm__ __volatile__ ("amoor.w.aqrl zero, zero, (%0)" : : "r" (&foo));
	DEBUG_print("SND CMD2\n");
	
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
	uint32_t foo = 0;
	__asm__ __volatile__ ("sfence.vma");
	__asm__ __volatile__ ("fence.i");
	__asm__ __volatile__ ("amoor.w.aqrl zero, zero, (%0)" : : "r" (&foo));
	DEBUG_print("SND CMD3\n");
	
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
