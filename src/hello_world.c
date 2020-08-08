typedef unsigned int uint32_t;

void cpu_relax() {
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	return;
}

void uart0_putc(char c) {
	void* uart_loc;
	volatile uint32_t* register_state;
	uart_loc = (void*)0x10013000;
	register_state = (uint32_t*)(uart_loc + 0x00);
	while (*register_state & 0x80000000) {
		cpu_relax();
	}
	*register_state = (uint32_t)c;
	return;
}

void uart1_putc(char c) {
	void* uart_loc;
	volatile uint32_t* register_state;
	uart_loc = (void*)0x10023000;
	register_state = (uint32_t*)(uart_loc + 0x00);
	while (*register_state & 0x80000000) {
		cpu_relax();
	}
	*register_state = (uint32_t)c;
	return;
}

void write(char* str) {
	if (str == 0) {
		return;
	}
	unsigned int i;
	i = 0;
	while (str[i] != 0) {
		uart0_putc(str[i]);
		uart1_putc(str[i]);
		i++;
	}
	return;
}

signed int main(unsigned int argc, char* argv[], char* envp[]) {
	// Enable TX on UARTs
	volatile uint32_t* uart_txctrl_reg;
	uart_txctrl_reg = (uint32_t*)(0x10013000 + 0x08);
	*uart_txctrl_reg = 0x1;
	uart_txctrl_reg = (uint32_t*)(0x10023000 + 0x08);
	*uart_txctrl_reg = 0x1;
	
	write("Hello, World!\n");
	__asm__ __volatile__ ("wfi");
	
	return 0;
}
