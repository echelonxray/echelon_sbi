#include "./context_switch.h"
#include "./../../inc/types.h"
#include "./../../inc/string.h"
#include "./../../inc/kernel_calls.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../kstart_entry.h"
#include "./../drivers/uart.h"
#include "./../globals.h"
#include "./../debug.h"

extern volatile CPU_Context* hart_contexts;
extern volatile Hart_Command* hart_commands;

extern __thread uintRL_t mhartid;

void* hart_start_c_handler(uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value) {
	mhartid = hart_contexts[hart_context_index].context_id;
	
	// Allow all memory access
	// QEMU will fail when switching to Supervisor mode of no PMP rules are set
	__asm__ __volatile__ ("csrw pmpaddr0, %0" : : "r" (0x003FFFFFFFFFFFFF));
	__asm__ __volatile__ ("csrw pmpcfg0, %0" : : "r" (0x000000000000000F));
	
	if (is_interrupt) {
		if (cause_value == 3) {
			// Machine Software Interrupt
			char buf[20];
			itoa(mhartid, buf, 20, -10, 0);
			DEBUG_print("Hart: ");
			DEBUG_print(buf);
			DEBUG_print(" - Started\n");
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
			volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
			clint_hart_msip_ctls[mhartid] = 0;
			if (hart_commands[mhartid].command == HARTCMD_SWITCHCONTEXT) {
				switch_context((CPU_Context*)(hart_commands[mhartid].param0));
			}
		}

		DEBUG_print("ESBI Interrupt!  Lower mcause bits: ");
		char str[30];
		itoa(cause_value, str, 30, 10, 0);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
	} else {
		// Exception caused handler to fire

		if (cause_value == 8) {
			// User-Mode Environment Exception
			DEBUG_print("ESBI Trap Caught!  From: U-Mode.  Trap Handler: M-Mode\n");
			cpu_context->regs[REG_PC] += 4;
			//csrs medeleg, a4
			//__asm__ __volatile__ ("csrs medeleg, %0" : : "r" (0x0000_0000_0000_0000));
			__asm__ __volatile__ ("csrs medeleg, %0" : : "r" (0x0000000000000100));
			
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
	
	return;
}
