#include "./init.h"
#include "./../../kernel/inc/memmap.h"
#include "./../../inc/types.h"
#include "./../../inc/kernel_calls.h"
//#include "./../../inc/string.c"

#define SYSCALL3_V(callnum, arg1, arg2, arg3) \
	{ register uintRL_t __a0 __asm__("a0") = (uintRL_t)callnum; \
	  register uintRL_t __a1 __asm__("a1") = (uintRL_t)arg1; \
	  register uintRL_t __a2 __asm__("a2") = (uintRL_t)arg2; \
	  register uintRL_t __a3 __asm__("a3") = (uintRL_t)arg3; \
	  __asm__ __volatile__ ("ecall\n" : : "r" (__a0), "r" (__a1), "r" (__a2), "r" (__a3) : "memory", "cc"); }

void uart_write(unsigned int uart_num, unsigned char* buf, size_t count) {
	SYSCALL3_V(KC_UARTWRITE, uart_num, buf, count);
}

signed int _start(unsigned int argc, char* argv[], char* envp[]) {
	//uart_write("Part #1!\n", UART0_BASE);
	//__asm__ __volatile__ ("ecall");
	unsigned char* str = "Hello, World!\n";
	uart_write(0, str, 14);
	//uart_write("Part #2!\n", UART0_BASE);
	while (1) {
		//write(1, "Hello, world!\n", 14);
	}
	return 0;
}
