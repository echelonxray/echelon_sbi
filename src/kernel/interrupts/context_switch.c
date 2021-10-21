#include "./context_switch.h"
#include "./../../inc/types.h"
#include "./../../inc/string.h"
#include "./../../inc/kernel_calls.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../thread_locking.h"
#include "./../sbi_commands.h"
#include "./../kstart_entry.h"
#include "./../drivers/uart.h"
#include "./../globals.h"
#include "./../debug.h"

extern ksemaphore_t* hart_command_que_locks;
extern volatile CPU_Context* hart_contexts;
extern volatile Hart_Command* hart_commands;

extern __thread uintRL_t mhartid;

void* hart_start_c_handler(uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value) {
	mhartid = hart_contexts[hart_context_index].context_id;
	
	/*
	// Allow all memory access
	// QEMU will fail when switching to Supervisor mode of no PMP rules are set
	__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" (0x003FFFFFFFFFFFFF));
	__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" (0x000000000000000F));
	*/
	
	if (is_interrupt) {
		if (cause_value == 3) {
			// Machine Software Interrupt
			volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
			clint_hart_msip_ctls[mhartid] = 0;
			return &interrupt_entry_handler;
		}
	}
	
	DEBUG_print("\n__Inside hart_start_c_handler()__\n");
	DEBUG_print("Unhanded Trap!  Spinning with codes: \n");
	char buf[20];
	
	itoa(mhartid, buf, 20, -10, 0);
	DEBUG_print("mhartid: ");
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(is_interrupt, buf, 20, -10, 0);
	DEBUG_print("is_interrupt: ");
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	itoa(cause_value, buf, 20, -10, 0);
	DEBUG_print("cause_value: ");
	DEBUG_print(buf);
	DEBUG_print("\n");
	
	idle_loop();
	return 0;
}

void interrupt_c_handler(volatile CPU_Context* cpu_context, uintRL_t cpu_context_index, uintRL_t is_interrupt, uintRL_t cause_value) {
	if (is_interrupt) {
		// Interrupt caused handler to fire
		
		if (cause_value == 3) {
			// M-Mode Software Interrupt -- This is a hart command
			
			// Save the command locally so that it does not get clobbered when the
			// msip flag is cleared.
			Hart_Command command = hart_commands[mhartid];
			
			volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
			
			if        (command.command == HARTCMD_SWITCHCONTEXT) {
				clint_hart_msip_ctls[mhartid] = 0;
				switch_context((CPU_Context*)(command.param0));
			} else if (command.command == HARTCMD_GETEXCEPTIONDELEGATION) {
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
			}
		} else {
			DEBUG_print("ESBI Interrupt!  Lower mcause bits: ");
			char str[30];
			itoa(cause_value, str, 30, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			idle_loop();
		}
	} else {
		// Exception caused handler to fire

		if (cause_value == 8) {
			// User-Mode Environment Exception
			DEBUG_print("ESBI Trap Caught!  From: U-Mode.  Trap Handler: M-Mode\n");
			cpu_context->regs[REG_PC] += 4;
		} else if (cause_value == 9) {
			// Supervisor-Mode Environment Exception
			DEBUG_print("ESBI Trap Caught!  From: S-Mode.  Trap Handler: M-Mode\n");
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
			DEBUG_print("ESBI Trap Caught!  From: M-Mode.  Trap Handler: M-Mode\n");
			cpu_context->regs[REG_PC] += 4;
		} else {
			DEBUG_print("ESBI Exception!  Lower mcause bits: ");
			char str[30];
			itoa(cause_value, str, 30, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			idle_loop();
		}
	}
	
	//idle_loop();
	return;
}

void send_hart_command_que(uintRL_t hart_id, Hart_Command* command) {
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	ksem_wait(hart_command_que_locks + hart_id);
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	ksem_post(hart_command_que_locks + hart_id);
	return;
}

void send_hart_command_blk(uintRL_t hart_id, Hart_Command* command) {
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	ksem_wait(hart_command_que_locks + hart_id);
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	ksem_post(hart_command_que_locks + hart_id);
	return;
}

void send_hart_command_ret(uintRL_t hart_id, Hart_Command* command) {
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	while (clint_hart_msip_ctls[hart_id]) {}
	ksem_wait(hart_command_que_locks + hart_id);
	hart_commands[hart_id] = *command;
	clint_hart_msip_ctls[hart_id] = 1;
	while (clint_hart_msip_ctls[hart_id]) {}
	*command = hart_commands[hart_id];
	ksem_post(hart_command_que_locks + hart_id);
	return;
}
