#include "./uart.h"
#include "./../inc/memmap.h"
#include "./../../inc/types.h"

void uart_putc(unsigned char c, void* uart_loc) {
	volatile uint8_t* register_state;
	register_state = (uint8_t*)(uart_loc + UART_TXDATA);
	//volatile uint32_t* register_state;
	//register_state = (uint32_t*)(uart_loc + UART_TXDATA);
	/*
	while (*register_state & 0x80000000) {
		CPU_RELAX();
	}
	*/
	*register_state = (uint32_t)c;
	if (c == '\n') {
		/*
		while (*register_state & 0x80000000) {
			CPU_RELAX();
		}
		*/
		*register_state = (uint32_t)'\r';
	}
	return;
}

void uart_write(const unsigned char* restrict str, void* uart_loc, size_t count) {
	if (str == 0) {
		return;
	}
	while (count > 0) {
		uart_putc(*str, uart_loc);
		str++;
		count--;
	}
	return;
}
