#include "./init.h"
#include "./../../kernel/inc/memmap.h"

signed int _start(unsigned int argc, char* argv[], char* envp[]) {
	//uart_write("Part #1!\n", UART0_BASE);
	__asm__ __volatile__ ("ecall");
	//uart_write("Part #2!\n", UART0_BASE);
	while (1) {
		//write(1, "Hello, world!\n", 14);
	}
	return 0;
}
