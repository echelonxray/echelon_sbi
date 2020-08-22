#include "./uart.h"
#include "./../inc/memmap.h"
#include "./../inc/general_oper.h"
#include "./../../inc/types.h"

void uart_putc(char c, void* uart_loc) {
	volatile uint32_t* register_state;
	register_state = (uint32_t*)(uart_loc + UART_TXDATA);
	while (*register_state & 0x80000000) {
		CPU_RELAX();
	}
	*register_state = (uint32_t)c;
	if (c == '\n') {
		while (*register_state & 0x80000000) {
			CPU_RELAX();
		}
		*register_state = (uint32_t)'\r';
	}
	return;
}

void uart_write(char* str, void* uart_loc) {
	if (str == 0) {
		return;
	}
	unsigned int i;
	i = 0;
	while (str[i] != 0) {
		uart_putc(str[i], uart_loc);
		i++;
	}
	return;
}
