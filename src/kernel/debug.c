#include "./debug.h"
#include "./drivers/uart.h"
#include "./inc/memmap.h"
#include "./../inc/string.h"

void DEBUG_print(char* str) {
	uart_write((unsigned char*)str, UART0_BASE, strlen(str));
	return;
}
