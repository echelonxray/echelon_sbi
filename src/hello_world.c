typedef unsigned int uint32_t;

void cpu_relax() {
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	__asm__ __volatile__ ("nop");
	return;
}

void uart_putc(char c, void* uart_loc) {
	volatile uint32_t* register_state;
	register_state = (uint32_t*)(uart_loc + 0x00);
	while (*register_state & 0x80000000) {
		cpu_relax();
	}
	*register_state = (uint32_t)c;
	if (c == '\n') {
		while (*register_state & 0x80000000) {
			cpu_relax();
		}
		*register_state = (uint32_t)'\r';
	}
	return;
}

void write(char* str) {
	if (str == 0) {
		return;
	}
	unsigned int i;
	i = 0;
	while (str[i] != 0) {
		uart_putc(str[i], (void*)0x10013000);
		uart_putc(str[i], (void*)0x10023000);
		i++;
	}
	return;
}

signed int main(unsigned int argc, char* argv[], char* envp[]) {
	volatile uint32_t* urat_reg;
	volatile uint32_t* ctrl_reg;
	volatile uint32_t* outp_reg;
	volatile uint32_t* prci_reg;
	
	// Setup the Clock
	prci_reg = (uint32_t*)(0x10008000 + 0x00);
	*prci_reg |= (1 << 30);
	prci_reg = (uint32_t*)(0x10008000 + 0x08);
	*prci_reg |= (1 << 16) | (1 << 17) | (1 << 18);
	prci_reg = (uint32_t*)(0x10008000 + 0x00);
	*prci_reg &= ~(1 << 30);
	
	// Set Div to 16MHz / (138 + 1) = ~115200 Baud
	urat_reg = (uint32_t*)(0x10013000 + 0x18);
	*urat_reg = 138;
	urat_reg = (uint32_t*)(0x10023000 + 0x18);
	*urat_reg = 138;
	
	// Disable UART Watermark Interrupts
	ctrl_reg = (uint32_t*)(0x10013000 + 0x10);
	*ctrl_reg = 0x0;
	ctrl_reg = (uint32_t*)(0x10023000 + 0x10);
	*ctrl_reg = 0x0;
	
	// Set GPIO Pins 0, 1, and 5 to output mode and turn them off
	ctrl_reg = (uint32_t*)(0x10012000 + 0x08);
	outp_reg = (uint32_t*)(0x10012000 + 0x0C);
	*ctrl_reg |=  (1 << 5);
	*ctrl_reg |=  (1 << 0);
	*ctrl_reg |=  (1 << 1);
	*outp_reg &= ~(1 << 5);
	*outp_reg &= ~(1 << 0);
	*outp_reg &= ~(1 << 1);
	
	// Set HW I/O UART functionality through the GPIO Pins (IOF) [These doesn't seem to be documented in the manual]uint32_t uart0_pins;
	uint32_t uart0_pins;
	uint32_t uart1_pins;
	uart0_pins = 0;
	uart0_pins |= (1 << 16);
	uart0_pins |= (1 << 17);
	uart1_pins = 0;
	uart1_pins |= (1 << 23);
	uart1_pins |= (1 << 18);
	ctrl_reg = (uint32_t*)(0x10012000 + 0x3C);
	*ctrl_reg &= ~(uart0_pins);
	*ctrl_reg &= ~(uart1_pins);
	ctrl_reg = (uint32_t*)(0x10012000 + 0x38);
	*ctrl_reg |=  (uart0_pins);
	*ctrl_reg |=  (uart1_pins);
	
	//ctrl_reg = (uint32_t*)(0x10013000 + 0x0C);
	//*ctrl_reg |= 0x1;
	//ctrl_reg = (uint32_t*)(0x10023000 + 0x0C);
	//*ctrl_reg |= 0x1;
	
	// Enable TX on UARTs
	ctrl_reg = (uint32_t*)(0x10013000 + 0x08);
	*ctrl_reg = 0x1;
	ctrl_reg = (uint32_t*)(0x10023000 + 0x08);
	*ctrl_reg = 0x1;
	
	//volatile int i = 0;
	//while(i++ < 10000000);
	//ctrl_reg = (uint32_t*)(0x10013000 + 0x08);
	
	//uint32_t clock_reg_val;
	//volatile uint32_t* clock_reg;
	//clock_reg = (uint32_t*)(0x10008000);
	
	//clock_reg_val = *clock_reg;
	//clock_reg_val = *urat_reg;
	//if (clock_reg_val == 868) {
	//if (clock_reg_val == 1437) {
	//if (((clock_reg_val & 0x1F0000) >> 16) == 16) {
	//if ((clock_reg_val & 0xC0000000) == 0xC0000000) {
	//if ((clock_reg_val & 0x3F) == 4) {
	//if (*ctrl_reg == 0x1) {
	//	*outp_reg |= (1 << 0);
		//*ctrl_reg = (1 << 0);
	//} else {
	//	*outp_reg |= (1 << 1);
		//*ctrl_reg = (1 << 1);
	//}
	
	// Turn ON the blue LED on the RED-V
	*outp_reg |=  (1 << 5);
	
	write("Hello, World!\n");
	__asm__ __volatile__ ("wfi");
	
	return 0;
}
