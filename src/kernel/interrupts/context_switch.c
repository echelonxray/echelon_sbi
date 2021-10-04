#include "./context_switch.h"
#include "./../../inc/types.h"
#include "./../../inc/string.h"
#include "./../../inc/kernel_calls.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../kstart_entry.h"
#include "./../drivers/uart.h"
#include "./../debug.h"

void interrupt_c_handler(CPU_Context* cpu_context) {
	//DEBUG_print("Trap Caught: ");
	
	//volatile uint32_t* ctrl_reg;
	sintRL_t mcause;
	__asm__ __volatile__ ("csrrc %0, mcause, zero" : "=r" (mcause));
	if (mcause < 0) {
		DEBUG_print("ESBI Error 1! Interrupt Triggered. Lower mcause bits: ");
		char str[20];
		mcause &= 0x7FFFFFFF;
		itoa(mcause, str, 20, 10, 0);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
	} else {
		if (mcause == 8) {
			// User Mode Environment Exception
			//DEBUG_print("User Mode Environment Exception!\n");
			uintRL_t Code = cpu_context->regs[10];
			uintRL_t Param1 = cpu_context->regs[11];
			uintRL_t Param2 = cpu_context->regs[12];
			uintRL_t Param3 = cpu_context->regs[13];
			if(Code == KC_UARTWRITE) {
				unsigned char* data = (unsigned char*)Param2;
				size_t count = (size_t)Param3;
				if (Param1 == 0) {
					// UART0
					uart_write(data, UART0_BASE, count);
				} else if (Param1 == 1) {
					// UART1
					uart_write(data, UART1_BASE, count);
				}
			}
		} else {
			DEBUG_print("ESBI Error 2! Not a User Mode Environment Exception. Lower mcause bits: ");
			char str[20];
			itoa(mcause, str, 20, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			idle_loop();
		}
	}

	return;
}
