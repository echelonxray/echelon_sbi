#include "./context_switch.h"
#include "./../../inc/types.h"
#include "./../../inc/string.h"
#include "./../../inc/kernel_calls.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../thread_locking.h"
#include "./../kstart_entry.h"
#include "./../drivers/uart.h"
#include "./../globals.h"
#include "./../debug.h"

extern ksemaphore_t* hart_command_que_locks;
extern volatile CPU_Context* hart_contexts;
extern volatile Hart_Command* hart_commands;

extern __thread uintRL_t mhartid;

#define SBI_SUCCESS 0
#define SBI_ERR_FAILED -1
#define SBI_ERR_NOT_SUPPORTED -2
#define SBI_ERR_INVALID_PARAM -3
#define SBI_ERR_DENIED -4
#define SBI_ERR_INVALID_ADDRESS -5
#define SBI_ERR_ALREADY_AVAILABLE -6
#define SBI_ERR_ALREADY_STARTED -7
#define SBI_ERR_ALREADY_STOPPED -8

void* hart_start_c_handler(uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value) {
	mhartid = hart_contexts[hart_context_index].context_id;
	
	// Allow all memory access
	// QEMU will fail when switching to Supervisor mode of no PMP rules are set
	__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" (0x003FFFFFFFFFFFFF));
	__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" (0x000000000000000F));
	
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
			//csrs medeleg, a4
			//__asm__ __volatile__ ("csrs medeleg, %0" : : "r" (0x0000_0000_0000_0000));
			//__asm__ __volatile__ ("csrs medeleg, %0" : : "r" (0x0000000000000100));
			
			/*
			// Get Request Code
			uintRL_t Request_Code = cpu_context->regs[REG_A0];

			// Get Request Parameters
			uintRL_t Param0 = cpu_context->regs[REG_A1];
			uintRL_t Param1 = cpu_context->regs[REG_A2];
			uintRL_t Param2 = cpu_context->regs[REG_A3];

			if(Request_Code == KC_UARTWRITE) {
				unsigned char* data = (unsigned char*)Param1;
				size_t count = (size_t)Param2;
				if (Param0 == 0) {
					// UART0
					uart_write(data, UART0_BASE, count);
				} else if (Param0 == 1) {
					// UART1
					uart_write(data, UART1_BASE, count);
				}
			}
			*/
		} else if (cause_value == 9) {
			// Supervisor-Mode Environment Exception
			DEBUG_print("ESBI Trap Caught!  From: S-Mode.  Trap Handler: M-Mode\n");
			cpu_context->regs[REG_PC] += 4;
			
			if (cpu_context->regs[REG_A7] == 0x10) {
				if        (cpu_context->regs[REG_A6] == 0) {
					// Get SBI specification version
					DEBUG_print("\tGet SBI specification version\n");
					cpu_context->regs[REG_A1] = 2;
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 1) {
					// Get SBI implementation ID
					DEBUG_print("\tGet SBI implementation ID\n");
					cpu_context->regs[REG_A1] = 6;
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 2) {
					// Get SBI implementation version
					DEBUG_print("\tGet SBI implementation version\n");
					cpu_context->regs[REG_A1] = 0;
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 3) {
					// Probe SBI extension
					DEBUG_print("\tProbe SBI extension: ");
					char str[30];
					itoa(cpu_context->regs[REG_A0], str, 30, -16, 0);
					DEBUG_print(str);
					DEBUG_print("\n");
					cpu_context->regs[REG_A1] = 0;
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 4) {
					// Get machine vendor ID
					DEBUG_print("\tGet machine vendor ID\n");
					__asm__ __volatile__ ("csrr %0, mvendorid" : "=r" (cpu_context->regs[REG_A1]));
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 5) {
					// Get machine architecture ID
					DEBUG_print("\tGet machine architecture ID\n");
					__asm__ __volatile__ ("csrr %0, marchid" : "=r" (cpu_context->regs[REG_A1]));
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else if (cpu_context->regs[REG_A6] == 6) {
					// Get machine implementation ID
					DEBUG_print("\tGet machine implementation ID\n");
					__asm__ __volatile__ ("csrr %0, mimpid" : "=r" (cpu_context->regs[REG_A1]));
					cpu_context->regs[REG_A0] = SBI_SUCCESS;
				} else {
					goto not_supported;
				}
			} else {
				// Not Supported
				not_supported:
				char str[30];
				DEBUG_print("\tError: ");
				itoa(cpu_context->regs[REG_A7], str, 30, -16, 0);
				DEBUG_print(str);
				DEBUG_print(" x ");
				itoa(cpu_context->regs[REG_A6], str, 30, -16, 0);
				DEBUG_print(str);
				DEBUG_print("\n");
				cpu_context->regs[REG_A0] = SBI_ERR_NOT_SUPPORTED;
			}
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
