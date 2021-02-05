#include "./uart.h"
#include "./../inc/memmap.h"
#include "./../inc/general_oper.h"
#include "./../../inc/types.h"

void uart_putc2(unsigned char c, void* uart_loc) {
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

void uart_putc(unsigned char c, void* uart_loc) {
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

void uart_write2(unsigned char* str, void* uart_loc, size_t count) {
	if (str == 0) {
		return;
	}
	while (count > 0) {
		__asm__ __volatile__ ("add zero, a1, a2");
		uart_putc2(*str, uart_loc);
		str++;
		count--;
	}
	return;
}

void uart_write(unsigned char* str, void* uart_loc, size_t count) {
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
