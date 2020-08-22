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
		mcause &= 0x7FFFFFFF;
		if (mcause == 3) {
			// Machine software interrupt
			DEBUG_print("Error 2!\n");
		} else if (mcause == 7) {
			// Machine timer interrupt
			//write("Bye, world!\n");

			// Disable the timer interrupt to prevent infinite loops
			DISABLE_TIMER_INTERRUPT();

			// mtime
			ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_LO); // Lower-half of 64-bit value
			*ctrl_reg = 0;
			ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_HI); // Upper-half of 64-bit value
			*ctrl_reg = 0;

			// rtc counter_start
			//ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTLO); // Low-Half
			//*ctrl_reg = 0;
			//ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTHI); // High-Half
			//*ctrl_reg = 0;
		} else if (mcause == 11) {
			// Machine external interrupt
			DEBUG_print("Error 3!\n");
		} else {
			// This shouldn't be reachable
			DEBUG_print("Error 0!\n");
		}
	} else {
		DEBUG_print("Error 1!\n");
	}

	return;
}
