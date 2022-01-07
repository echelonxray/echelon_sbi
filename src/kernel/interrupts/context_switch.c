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
extern uintRL_t load_point;

extern __thread uintRL_t mhartid;

void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value) {
	char str[30];
	//DEBUG_print("interrupt_c_handler\n");
	
	if        (cause_value == 3) {
		// M-Mode Software Interrupt -- This is a hart command

		// Save the command locally so that it does not get clobbered when the
		// msip flag is cleared.
		Hart_Command command = hart_commands[mhartid];

		volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;

		if        (command.command == HARTCMD_SWITCHCONTEXT) {
			clint_hart_msip_ctls[mhartid] = 0;
			switch_context((CPU_Context*)(command.param0));
		} /* else if (command.command == HARTCMD_GETEXCEPTIONDELEGATION) {
			__asm__ __volatile__ ("csrr %0, medeleg" : "=r" (hart_commands[mhartid].param0));
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETEXCEPTIONDELEGATION) {
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrw medeleg, %0" : : "r" (command.param0));
		} else if (command.command == HARTCMD_GETINTERRUPTDELEGATION) {
			__asm__ __volatile__ ("csrr %0, mideleg" : "=r" (hart_commands[mhartid].param0));
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETINTERRUPTDELEGATION) {
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrw mideleg, %0" : : "r" (command.param0));
		} else if (command.command == HARTCMD_GETPMPADDR) {
			if        (command.param1 ==  0) {
				__asm__ __volatile__ ("csrr %0, pmpaddr0 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  1) {
				__asm__ __volatile__ ("csrr %0, pmpaddr1 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  2) {
				__asm__ __volatile__ ("csrr %0, pmpaddr2 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  3) {
				__asm__ __volatile__ ("csrr %0, pmpaddr3 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  4) {
				__asm__ __volatile__ ("csrr %0, pmpaddr4 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  5) {
				__asm__ __volatile__ ("csrr %0, pmpaddr5 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  6) {
				__asm__ __volatile__ ("csrr %0, pmpaddr6 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  7) {
				__asm__ __volatile__ ("csrr %0, pmpaddr7 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  8) {
				__asm__ __volatile__ ("csrr %0, pmpaddr8 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  9) {
				__asm__ __volatile__ ("csrr %0, pmpaddr9 " : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 10) {
				__asm__ __volatile__ ("csrr %0, pmpaddr10" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 11) {
				__asm__ __volatile__ ("csrr %0, pmpaddr11" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 12) {
				__asm__ __volatile__ ("csrr %0, pmpaddr12" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 13) {
				__asm__ __volatile__ ("csrr %0, pmpaddr13" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 14) {
				__asm__ __volatile__ ("csrr %0, pmpaddr14" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 == 15) {
				__asm__ __volatile__ ("csrr %0, pmpaddr15" : "=r" (hart_commands[mhartid].param0));
			}
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETPMPADDR) {
			clint_hart_msip_ctls[mhartid] = 0;
			if        (command.param1 ==  0) {
				__asm__ __volatile__ ("csrw pmpaddr0, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  1) {
				__asm__ __volatile__ ("csrw pmpaddr1, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  2) {
				__asm__ __volatile__ ("csrw pmpaddr2, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  3) {
				__asm__ __volatile__ ("csrw pmpaddr3, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  4) {
				__asm__ __volatile__ ("csrw pmpaddr4, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  5) {
				__asm__ __volatile__ ("csrw pmpaddr5, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  6) {
				__asm__ __volatile__ ("csrw pmpaddr6, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  7) {
				__asm__ __volatile__ ("csrw pmpaddr7, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  8) {
				__asm__ __volatile__ ("csrw pmpaddr8, %0 " : : "r" (command.param0));
			} else if (command.param1 ==  9) {
				__asm__ __volatile__ ("csrw pmpaddr9, %0 " : : "r" (command.param0));
			} else if (command.param1 == 10) {
				__asm__ __volatile__ ("csrw pmpaddr10, %0" : : "r" (command.param0));
			} else if (command.param1 == 11) {
				__asm__ __volatile__ ("csrw pmpaddr11, %0" : : "r" (command.param0));
			} else if (command.param1 == 12) {
				__asm__ __volatile__ ("csrw pmpaddr12, %0" : : "r" (command.param0));
			} else if (command.param1 == 13) {
				__asm__ __volatile__ ("csrw pmpaddr13, %0" : : "r" (command.param0));
			} else if (command.param1 == 14) {
				__asm__ __volatile__ ("csrw pmpaddr14, %0" : : "r" (command.param0));
			} else if (command.param1 == 15) {
				__asm__ __volatile__ ("csrw pmpaddr15, %0" : : "r" (command.param0));
			}
		} else if (command.command == HARTCMD_GETPMPCFG) {
			if        (command.param1 ==  0) {
				__asm__ __volatile__ ("csrr %0, pmpcfg0" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  1) {
				__asm__ __volatile__ ("csrr %0, pmpcfg1" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  2) {
				__asm__ __volatile__ ("csrr %0, pmpcfg2" : "=r" (hart_commands[mhartid].param0));
			} else if (command.param1 ==  3) {
				__asm__ __volatile__ ("csrr %0, pmpcfg3" : "=r" (hart_commands[mhartid].param0));
			}
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETPMPCFG) {
			clint_hart_msip_ctls[mhartid] = 0;
			if        (command.param1 ==  0) {
				__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" (command.param0));
			} else if (command.param1 ==  1) {
				__asm__ __volatile__ ("csrw pmpcfg1, %0" : : "r" (command.param0));
			} else if (command.param1 ==  2) {
				__asm__ __volatile__ ("csrw pmpcfg2, %0" : : "r" (command.param0));
			} else if (command.param1 ==  3) {
				__asm__ __volatile__ ("csrw pmpcfg3, %0" : : "r" (command.param0));
			}
		} else if (command.command == HARTCMD_GETSATP) {
			__asm__ __volatile__ ("csrr %0, satp" : "=r" (hart_commands[mhartid].param0));
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETSATP) {
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrw satp, %0" : : "r" (command.param0));
		} else if (command.command == HARTCMD_GETSSTATUS) {
			__asm__ __volatile__ ("csrr %0, sstatus" : "=r" (hart_commands[mhartid].param0));
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SETSSTATUS) {
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrw sstatus, %0" : : "r" (command.param0));
		} */ else if (command.command == HARTCMD_STARTHART) {
			DEBUG_print("\tStarted: ");
			itoa(mhartid, str, 30, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			//__asm__ __volatile__ ("csrs mie, %0" : : "r" (1 << 7));
			
#ifdef MM_FU540_C000
			__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" (0x0000000080000000));
			__asm__ __volatile__ ("csrw pmpaddr1, %0" : : "r" (load_point));
			__asm__ __volatile__ ("csrw pmpaddr2, %0" : : "r" (0x003FFFFFFFFFFFFF));
			__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" ((0x0F << 16) | (0x08 <<  8) | (0x0F <<  0)));
#endif
			
			//uintRL_t delegation;
			/*
			delegation  = 0;
			delegation |= (1 <<  0) | (1 <<  1) | (0 <<  2) | (1 <<  3) | (1 <<  4) | (1 <<  5) | (1 <<  6);
			delegation |= (1 <<  7) | (1 <<  8) |                                     (1 << 12) | (1 << 13);
			delegation |=             (1 << 15);
			*/
			//delegation = 0;
			//delegation = (1 << 12);
			__asm__ __volatile__ ("csrw medeleg, %0" : : "r" (0x000));
			//delegation = 0;
			//delegation |= (1 <<  0) | (1 <<  1) |                         (1 <<  4) | (1 <<  5)            ;
			//delegation |=             (1 <<  8) | (1 <<  9);
			//delegation = 0;
			//delegation |= (1 <<  1) | (1 <<  5);
			__asm__ __volatile__ ("csrw mideleg, %0" : : "r" (0x222));
			
			//__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x22));
			//__asm__ __volatile__ ("csrw mie, zero");
			__asm__ __volatile__ ("csrw mie, %0"     : : "r" (0x02A));
			
			__asm__ __volatile__ ("csrw satp, zero");
			__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x0A));
			__asm__ __volatile__ ("csrs mstatus, %0" : : "r" (0x80));
			
			clear_hart_context(hart_contexts_user + mhartid);
			hart_contexts_user[mhartid].context_id = mhartid;
			hart_contexts_user[mhartid].execution_mode = EM_S;
			hart_contexts_user[mhartid].regs[REG_PC] = command.param1;
			hart_contexts_user[mhartid].regs[REG_A0] = command.param0;
			hart_contexts_user[mhartid].regs[REG_A1] = command.param2;
			
			//ksem_wait(sbi_hsm_locks + mhartid);
			sbi_hsm_states[mhartid] = SBI_HSM_STARTED;
			//ksem_post(sbi_hsm_locks + mhartid);
			
			clint_hart_msip_ctls[mhartid] = 0;
			//DEBUG_print("Started\n");
			switch_context(hart_contexts_user + mhartid);
		} else if (command.command == HARTCMD_REMOTE_FENCE_I) {
			__asm__ __volatile__ ("fence.i");
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA) {
			// TODO: Specific parameters
			__asm__ __volatile__ ("sfence.vma zero, zero");
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_REMOTE_SFENCE_VMA_ASID) {
			// TODO: Specific parameters
			__asm__ __volatile__ ("sfence.vma zero, zero");
			clint_hart_msip_ctls[mhartid] = 0;
		} else if (command.command == HARTCMD_SMODE_SOFTINT) {
			clint_hart_msip_ctls[mhartid] = 0;
			__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x2));
			/*
			DEBUG_print("\tHARTCMD_SMODE_SOFTINT [Hart: ");
			itoa(mhartid, str, 30, 10, 0);
			DEBUG_print(str);
			DEBUG_print("]\n");
			*/
		} else {
			goto not_handled_interrupt;
		}
	} else if (cause_value == 7) {
		// M-Mode Timer Interrupt
		
		/*
		char buf[20];
		DEBUG_print("\tM-Mode Timer Int received in M-Mode.  mhartid: ");
		itoa(mhartid, buf, 20, 10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		*/
		
		__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x80));
		__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x20));
	} else if (cause_value == 11) {
		// M-Mode External Interrupt
		
		DEBUG_print("M-Mode External Int received in M-Mode\n");
		__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x80));
		//__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x20));
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

void exception_c_handler(volatile CPU_Context* cpu_context, uintRL_t cause_value) {
	//char str[30];
	//DEBUG_print("exception_c_handler\n");
	
	if (cpu_context->execution_mode == 3) {
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		DEBUG_print("ESBI Trap Caught!  Exception!  From: M-Mode.  Trap Handler: M-Mode\n");
		
		char buf[20];
		memset(buf, 0, 20);
		DEBUG_print("ESBI Exception!  Lower mcause bits: ");
		itoa(cause_value, buf, 20, 10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tHart ID: ");
		itoa(mhartid, buf, 20, -10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], buf, 20, -16, -8);
		DEBUG_print(buf);
		DEBUG_print("\n");
		uint32_t* inst = (uint32_t*)(cpu_context->regs[REG_PC]);
		DEBUG_print("\tINST: 0x");
		itoa(*inst, buf, 20, -16, -8);
		DEBUG_print(buf);
		DEBUG_print("\n");
		
		DEBUG_print("\n");
		
		DEBUG_print("\t x1: ");
		itoa(cpu_context->regs[REG_X1], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x2: ");
		itoa(cpu_context->regs[REG_X2], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x3: ");
		itoa(cpu_context->regs[REG_X3], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x4: ");
		itoa(cpu_context->regs[REG_X4], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x5: ");
		itoa(cpu_context->regs[REG_X5], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x6: ");
		itoa(cpu_context->regs[REG_X6], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x7: ");
		itoa(cpu_context->regs[REG_X7], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x8: ");
		itoa(cpu_context->regs[REG_X8], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\t x9: ");
		itoa(cpu_context->regs[REG_X9], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx10: ");
		itoa(cpu_context->regs[REG_X10], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx11: ");
		itoa(cpu_context->regs[REG_X11], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx12: ");
		itoa(cpu_context->regs[REG_X12], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx13: ");
		itoa(cpu_context->regs[REG_X13], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx14: ");
		itoa(cpu_context->regs[REG_X14], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx15: ");
		itoa(cpu_context->regs[REG_X15], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx16: ");
		itoa(cpu_context->regs[REG_X16], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx17: ");
		itoa(cpu_context->regs[REG_X17], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx18: ");
		itoa(cpu_context->regs[REG_X18], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx19: ");
		itoa(cpu_context->regs[REG_X19], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx20: ");
		itoa(cpu_context->regs[REG_X20], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx21: ");
		itoa(cpu_context->regs[REG_X21], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx22: ");
		itoa(cpu_context->regs[REG_X22], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx23: ");
		itoa(cpu_context->regs[REG_X23], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx24: ");
		itoa(cpu_context->regs[REG_X24], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx25: ");
		itoa(cpu_context->regs[REG_X25], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx26: ");
		itoa(cpu_context->regs[REG_X26], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx27: ");
		itoa(cpu_context->regs[REG_X27], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx28: ");
		itoa(cpu_context->regs[REG_X28], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx29: ");
		itoa(cpu_context->regs[REG_X29], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx30: ");
		itoa(cpu_context->regs[REG_X30], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tx31: ");
		itoa(cpu_context->regs[REG_X31], buf, 20, -16, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		
		idle_loop();
	}
	
	if        (cause_value == 0) {
		// Instruction Address Misaligned
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 1) {
		// Instruction Access Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 2) {
		// Illegal Instruction
		uintRL_t csr_satp;
		__asm__ __volatile__ ("csrr %0, satp" : "=r" (csr_satp));
		uint32_t* instruction = (void*)walk_pts(cpu_context->regs[REG_PC], csr_satp);
		dec_inst dinst;
		uintRL_t form = decode_instruction(*instruction, &dinst);
		if (form) {
			//DEBUG_print("TraceA\n");
			/*
			DEBUG_print("\n\topcode: ");
			itoa(dinst.opcode, str, 30, -16, -8);
			DEBUG_print(str);
			DEBUG_print("\n\trd: ");
			itoa(dinst.rd, str, 30, -16, -8);
			DEBUG_print(str);
			DEBUG_print("\n\tfunct3: ");
			itoa(dinst.funct3, str, 30, -16, -8);
			DEBUG_print(str);
			DEBUG_print("\n\trs1: ");
			itoa(dinst.rs1, str, 30, -16, -8);
			DEBUG_print(str);
			DEBUG_print("\n\timm: ");
			itoa(dinst.imm, str, 30, -16, -8);
			DEBUG_print(str);
			*/
			if (dinst.opcode == 0x73) {
				if (dinst.funct3 == 0x2 || dinst.funct3 == 0x3 || dinst.funct3 == 0x6 || dinst.funct3 == 0x7) {
					if (dinst.rs1 == 0 && dinst.rd != 0) {
						if (dinst.imm == 0xC01) {
							//DEBUG_print("TraceB\n");
							uint64_t* mtime = (void*)(CLINT_BASE + CLINT_MTIME);
							cpu_context->regs[dinst.rd] = *mtime;
							cpu_context->regs[REG_PC] += 4;
							//DEBUG_print("Return\n");
							switch_context(cpu_context);
						}
					}
				}
			}
		}
		s_delegation_trampoline(cpu_context, 0, *instruction);
		/*
		DEBUG_print("[Hart: ");
		itoa(mhartid, str, 30, 10, 0);
		DEBUG_print(str);
		DEBUG_print("] ");
		DEBUG_print("ESBI Exception!  Illegal instruction.\n");
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], str, 30, -16, -8);
		DEBUG_print(str);
		DEBUG_print("\n");
		DEBUG_print("\tOpcode 4-Byte Value: 0x");
		itoa(*instruction, str, 30, -16, -8);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
		*/
	} else if (cause_value == 3) {
		// Breakpoint
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		DEBUG_print("ESBI Trap Caught!  Breakpoint Exception.  Trap Handler: M-Mode\n");
		idle_loop();
	} else if (cause_value == 4) {
		// Load Address Misaligned
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 5) {
		// Load Access Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 6) {
		// Store/AMO Address Misaligned
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 7) {
		// Store/AMO Access Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 8) {
		// User-Mode Environment Exception
		/*
		DEBUG_print("ESBI Trap Caught!  From: U-Mode.  Trap Handler: M-Mode\n");
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], str, 30, -16, -8);
		DEBUG_print(str);
		DEBUG_print("\n");
		*/
		cpu_context->regs[REG_PC] += 4;
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 9) {
		// Supervisor-Mode Environment Exception
		/*
		DEBUG_print("\tESBI Trap Caught!  From: S-Mode.  Trap Handler: M-Mode\n");
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], str, 30, -16, -8);
		DEBUG_print(str);
		DEBUG_print("\n");
		*/
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
	} else if (cause_value == 11) {
		// Machine-Mode Environment Exception
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		DEBUG_print("ESBI Trap Caught!  Machine-Mode Environment Exception.  Trap Handler: M-Mode. mhartid: \n");
		char buf[20];
		itoa(mhartid, buf, 20, -10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		idle_loop();
		/*
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], str, 30, -16, -8);
		DEBUG_print(str);
		DEBUG_print("\n");
		*/
		cpu_context->regs[REG_PC] += 4;
	} else if (cause_value == 12) {
		// Instruction Page Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 13) {
		// Load Page Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else if (cause_value == 15) {
		// Store/AMO Page Fault
		s_delegation_trampoline(cpu_context, 0, 0);
	} else {
		__asm__ __volatile__ ("csrc mstatus, %0" : : "r" (0x8));
		char buf[20];
		memset(buf, 0, 20);
		DEBUG_print("ESBI Exception!  Lower mcause bits: ");
		itoa(cause_value, buf, 20, 10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tHart ID: ");
		itoa(mhartid, buf, 20, -10, 0);
		DEBUG_print(buf);
		DEBUG_print("\n");
		DEBUG_print("\tPC: 0x");
		itoa(cpu_context->regs[REG_PC], buf, 20, -16, -8);
		DEBUG_print(buf);
		DEBUG_print("\n");
		idle_loop();
		//s_delegation_trampoline(cpu_context, 0, 0);
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
	//DEBUG_Print("Trace Start");
	memcpy(&params, &einst, sizeof(uint32_t));
	//DEBUG_print(" -- End\n");
	
	if (opcode == 0x73) {
		// OpCode: SYSTEM, Encoding: I-Type
		
		dinst->opcode = opcode;
		dinst->rd = params.enc_i.rd;
		dinst->funct3 = params.enc_i.funct3;
		dinst->rs1 = params.enc_i.rs1;
		dinst->imm = params.enc_i.imm;
		
		dinst->rs2 = 0;
		dinst->funct7 = 0;
		return 2;
	}
	
	return 0;
}

// This function assumes the PT structure is valid.  Do not call it if that is not guaranteed.
// This function assumes the PT structures are not changed (Such as by other cores).
// This function makes no security checks.  It assumes PTs structure is safe.  A malicious
// kernel could exploit it.
uintRL_t walk_pts(uintRL_t location, uintRL_t csr_satp) {
	uintRL_t mem_addr;
	
#if   __riscv_xlen == 128
	// 128-Bit XLEN
	
	#error "Virtual Memory parsing is not implemented for 128-bit XLEN"
	
#elif __riscv_xlen == 64
	// 64-Bit XLEN
	
	sint64_t page_walk = (sint64_t)csr_satp;
	// Is Virtual Memory Active?
	if (((uintRL_t)page_walk >> 60) == 8) { // Sv39
		// Shift to match PTE entry offset to ready for entry to the PT Walk loop
		page_walk <<= 10;
		
		// Walk the PTs
		uintRL_t shift_ammount = 9 + 9 + 12;
		do {
			// Correct Offset: Left Shift and then Right Arithmetic Shift for Sign Extension
			page_walk <<= 10;
			page_walk >>= 8;
			
			sint64_t* page_ptr = (sint64_t*)(page_walk & ~((uintRL_t)0xFFF));
			page_walk = page_ptr[(location >> shift_ammount) & 0x1FF];
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
	} else {
		mem_addr = (uintRL_t)location;
	}
	
#else
	// 32-Bit XLEN
	
	// Get the inital value of the satp CSR
	sint32_t page_walk = (sint32_t)csr_satp;
	
	// Is Virtual Memory Active?
	if (((uintRL_t)page_walk >> 31) == 1) {
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
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	//DEBUG_Print("TRACE_K\n");
	ksem_post(hart_command_que_locks + hart_id);
	//DEBUG_Print("TRACE_L\n");
	return;
}

void send_hart_command_lck(uintRL_t hart_id, Hart_Command* command) {
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	return;
}

void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command) {
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	//DEBUG_Print("TRACE_I\n");
	ksem_post(hart_command_que_locks + hart_id);
	//DEBUG_Print("TRACE_J\n");
	return;
}

void send_hart_command_ret(uintRL_t hart_id, Hart_Command* command) {
	ksem_wait(hart_command_que_locks + hart_id);
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	*command = hart_commands[hart_id];
	//DEBUG_Print("TRACE_G\n");
	ksem_post(hart_command_que_locks + hart_id);
	//DEBUG_Print("TRACE_H\n");
	return;
}
