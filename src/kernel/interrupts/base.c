#include "./base.h"
#include "./../../inc/types.h"
#include "./../inc/general_oper.h"
#include "./../inc/memmap.h"
#include "./../debug.h"

void interrupt_chandle() {
	//write("Trap Caught!\n");

	volatile uint32_t* ctrl_reg;
	uint32_t mcause;
	__asm__ __volatile__ ("csrrc %0, mcause, zero" : "=r" (mcause));
	if (mcause & 0x80000000) {
		DEBUG_print("Error 1! ");
		char str[20];
		itoa(mcause, str, 20, 10, 0);
		DEBUG_print(str);
		DEBUG_print("\n");
		idle_loop();
	} else {
		mcause &= 0x7FFFFFFF;
		if (mcause == 11) {
			// Machine Environment Exception
			DEBUG_print("!!--TEST OUTPUT--!!\n");
			__asm__ __volatile__ ("csrrc %0, mepc, zero" : "=r" (mcause));
			mcause += 4;
			__asm__ __volatile__ ("csrrw zero, mepc, %0" : : "r" (mcause));
		} else {
			DEBUG_print("Error 2! ");
			char str[20];
			itoa(mcause, str, 20, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			idle_loop();
		}
	}

	return;
}
