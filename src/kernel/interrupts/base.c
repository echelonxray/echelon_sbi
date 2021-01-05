#include "./base.h"
#include "./../../inc/types.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../interrupts/context_switch.h"
#include "./../debug.h"

void interrupt_chandle(CPU_Context* cpu_context) {
	DEBUG_print("Trap Caught!\n");
	
	volatile uint32_t* ctrl_reg;
	uint32_t mcause;
	__asm__ __volatile__ ("csrrc %0, mcause, zero" : "=r" (mcause));
	if (mcause & 0x80000000) {
		DEBUG_print("Error 1! Interrupt Triggered ");
		char str[20];
		itoa(mcause, str, 20, 10, 0);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
	} else {
		mcause &= 0x7FFFFFFF;
		if (mcause == 8) {
			// User Mode Environment Exception
			DEBUG_print("User Mode Environment Exception!\n");
		} else {
			DEBUG_print("Error 2! Not a User Mode Environment Exception ");
			char str[20];
			itoa(mcause, str, 20, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			idle_loop();
		}
	}

	return;
}
