#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./globals.h"
#include "./debug.h"

signed int kmain(unsigned int argc, char* argv[], char* envp[]) {
	// START: CPU Init
	volatile uint32_t* ctrl_reg;
	
	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;
	
	DEBUG_print("Hello World!\n");
	
	return 0;
}
