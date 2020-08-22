#include "./debug.h"
#include "./drivers/uart.h"
#include "./inc/memmap.h"

void DEBUG_print(char* str) {
	uart_write(str, UART0_BASE);
	return;
}
