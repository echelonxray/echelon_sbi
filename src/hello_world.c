typedef unsigned int uint32_t;

#define rck 2
#define srck 3
#define s_in 4

void interrupt_handler();

void cpu_wait() {
	__asm__ __volatile__ ("wfi");
	return;
}

void enable_timer_interrupt() {
	__asm__ __volatile__ ("csrrs zero, mie, %0" : : "r" (0x00000080));
	return;
}

void disable_timer_interrupt() {
	__asm__ __volatile__ ("csrrc zero, mie, %0" : : "r" (0x00000080));
	return;
}

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
		//uart_putc(str[i], (void*)0x10023000);
		i++;
	}
	return;
}

void itoa(register signed int number, register char* buf, register signed int buf_len, register signed int base, register signed int set_width) {
	register signed long int num;
	num = number;
	if (base < 0) {
		base = -base;
		if(number < 0) {
			num = -num;
		}
	}
	if (base > 16 || base < 2) {
		return;
	}
	if (buf_len < 2) {
		return;
	}
	if (set_width != 0) {
		if (set_width < 0 || num >= 0) {
			if (buf_len <= set_width) {
				return;
			}
		} else {
			if (buf_len <= set_width + 1) {
				return;
			}
		}
	}
	//Check if zero since this will fail to loop and can be easily handled now
	register unsigned char i_2;
	if (num == 0) {
		if (set_width != 0) {
			if (set_width < 0) {
				set_width = -set_width;
			}
			i_2 = 0;
			while (i_2 < set_width) {
				buf[i_2] = '0';
				i_2++;
			}
			buf[i_2] = 0;
			return;
		} else {
			buf[0] = '0';
			buf[1] = 0;
			return;
		}
	} else {
		register unsigned char i_then_length;
		i_then_length = 0;
		i_2 = 0;
		//i_then_length is now an index
		//Append "-" character for negatives
		if(num < 0){
			if (set_width < 0) {
				set_width++;
			}
			num = -num;
			buf[0] = '-';
			buf_len--;
			buf++;
		}
		if (set_width < 0) {
			set_width = -set_width;
		}
		//Find Characters
		while (num > 0 && i_then_length < buf_len) {
			i_2 = num % base;
			if (i_2 < 10) {
				buf[(unsigned int)i_then_length] = '0' + i_2;
			} else {
				buf[(unsigned int)i_then_length] = '7' + i_2;
			}
			num /= base;
			i_then_length++;
		}
		while (i_then_length < set_width && i_then_length < buf_len) {
			buf[(unsigned int)i_then_length] = '0';
			i_then_length++;
		}
		//i_then_length is now a length count for char array
		//Loop to fix character order
		i_2 = 0;
		register char tmpchar;
		while (i_2 < (i_then_length / 2) && i_2 < buf_len) {
			tmpchar = buf[(int)((i_then_length - i_2) - 1)];
			buf[(int)((i_then_length - i_2) - 1)] = buf[(unsigned int)i_2];
			buf[(unsigned int)i_2] = tmpchar;
			i_2++;
		}
		if (i_then_length < buf_len) {
			buf[(unsigned int)i_then_length] = 0;
		} else {
			buf[(unsigned int)(i_then_length - 1)] = 0;
		}
	}
	return;
}

void interrupt_chandle() {
	//write("Trap Caught!\n");

	volatile uint32_t* ctrl_reg;
	uint32_t mcause;
	__asm__ __volatile__ ("csrrc %0, mcause, zero" : "=r" (mcause));
	if (mcause & 0x80000000) {
		mcause &= 0x7FFFFFFF;
		if (mcause == 3) {
			// Machine software interrupt
			write("Error 2!\n");
		} else if (mcause == 7) {
			// Machine timer interrupt
			//write("Bye, world!\n");

			// Disable the timer interrupt to prevent infinite loops
			disable_timer_interrupt();

			// mtime
			ctrl_reg = (uint32_t*)(0x0200BFF8); // Lower-half of 64-bit value
			*ctrl_reg = 0;
			ctrl_reg = (uint32_t*)(0x0200BFFC); // Upper-half of 64-bit value
			*ctrl_reg = 0;

			// rtc counter_start
			ctrl_reg = (uint32_t*)(0x10000000 + 0x48); // Low-Half
			*ctrl_reg = 0;
			ctrl_reg = (uint32_t*)(0x10000000 + 0x4C); // High-Half
			*ctrl_reg = 0;
		} else if (mcause == 11) {
			// Machine external interrupt
			write("Error 3!\n");
		} else {
			// This shouldn't be reachable
			write("Error 0!\n");
		}
	} else {
		write("Error 1!\n");
	}

	return;
}

void low(volatile uint32_t* gpio_pins, unsigned int pin_num) {
	*gpio_pins = *gpio_pins & ~(1 << pin_num);
	return;
}
void high(volatile uint32_t* gpio_pins, unsigned int pin_num) {
	*gpio_pins = *gpio_pins | (1 << pin_num);
	return;
}
void pause() {
	cpu_wait();
	enable_timer_interrupt();
}

void write_shift_regs(volatile uint32_t* gpio_pins, unsigned char* regs, unsigned int len) {
	unsigned int pin_shft_serial_out;
	unsigned int pin_shft_serial_clk;
	unsigned int pin_shft_update_out;
	uint32_t gpio_state;
	uint32_t pin;
	pin_shft_serial_out = 4;
	pin_shft_serial_clk = 3;
	pin_shft_update_out = 2;
	gpio_state = *gpio_pins;

	unsigned int i;
	i = 0;
	while (i < len) {
		unsigned char byte_bits;
		byte_bits = *regs;
		unsigned int state;
		gpio_state = *gpio_pins;
		pin = 1 << pin_shft_serial_out;
		if (byte_bits & (1 << i)) {
			gpio_state = gpio_state | pin;
		} else {
			gpio_state = gpio_state & ~pin;
		}
		*gpio_pins = gpio_state;
		cpu_wait();
		enable_timer_interrupt();
		//*gpio_pins = (*gpio_pins & (~(1 << pin))) | ((*gpio_pins & (1 << pin)) ^ (1 << pin));
		pin = 1 << pin_shft_serial_clk;
		gpio_state = gpio_state | pin;
		*gpio_pins = gpio_state;
		cpu_wait();
		enable_timer_interrupt();
		gpio_state = gpio_state & ~pin;
		*gpio_pins = gpio_state;
		//*gpio_pins = (*gpio_pins & (~(1 << pin))) | ((*gpio_pins & (1 << pin)) ^ (1 << pin));
		//*gpio_pins = (*gpio_pins & (~(1 << pin))) | ((*gpio_pins & (1 << pin)) ^ (1 << pin));
		i++;
		if (i == 8) {
			i = 0;
			len -= 8;
		}
	}

	pin = 1 << pin_shft_update_out;
	cpu_wait();
	enable_timer_interrupt();
	gpio_state = gpio_state | pin;
	*gpio_pins = gpio_state;
	cpu_wait();
	enable_timer_interrupt();
	gpio_state = gpio_state & ~pin;
	*gpio_pins = gpio_state;

	return;
}

void write_command(uint32_t* gpio_pins, unsigned char command) {
	unsigned char regs[2];
	regs[0] = command;
	//regs[1] = 0b;
}
void write_data(uint32_t* gpio_pins, unsigned char data) {
	unsigned char regs[2];
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
	*prci_reg = (*prci_reg & 0x7FFFF000) | 0x000005F1;
	*prci_reg &= ~(1 << 18);
	prci_reg = (uint32_t*)(0x10008000 + 0x00);
	*prci_reg &= ~(1 << 30);

	// Set Div to 16MHz / (138 + 1) = ~115200 Baud
	urat_reg = (uint32_t*)(0x10013000 + 0x18);
	*urat_reg = 2221;
	urat_reg = (uint32_t*)(0x10023000 + 0x18);
	*urat_reg = 2221;

	// Disable UART Watermark Interrupts
	ctrl_reg = (uint32_t*)(0x10013000 + 0x10);
	*ctrl_reg = 0x0;
	ctrl_reg = (uint32_t*)(0x10023000 + 0x10);
	*ctrl_reg = 0x0;

	// Set GPIO Pins 0, 1, and 5 to output mode and turn them off
	ctrl_reg = (uint32_t*)(0x10012000 + 0x08);
	outp_reg = (uint32_t*)(0x10012000 + 0x0C);
	*ctrl_reg |=  (1 << 5);
	*ctrl_reg |=  (1 << 4);
	*ctrl_reg |=  (1 << 3);
	*ctrl_reg |=  (1 << 2);
	*outp_reg &= ~(1 << 5);
	*outp_reg &= ~(1 << 4);
	*outp_reg &= ~(1 << 3);
	*outp_reg &= ~(1 << 2);

	// Set HW I/O UART functionality through the GPIO Pins (IOF) [These doesn't seem to be documented in the manual]
	uint32_t uart0_pins;
	uint32_t uart1_pins;
	uart0_pins = 0;
	uart0_pins |= (1 << 16);
	uart0_pins |= (1 << 17);
	uart1_pins = 0;
	//uart1_pins |= (1 << 23);
	//uart1_pins |= (1 << 18);
	ctrl_reg = (uint32_t*)(0x10012000 + 0x3C);
	*ctrl_reg &= ~(uart0_pins);
	*ctrl_reg &= ~(uart1_pins);
	ctrl_reg = (uint32_t*)(0x10012000 + 0x38);
	*ctrl_reg =  0x00000000;
	*ctrl_reg |=  (uart0_pins);
	*ctrl_reg |=  (uart1_pins);

	// Enable TX on UARTs
	ctrl_reg = (uint32_t*)(0x10013000 + 0x08);
	*ctrl_reg = 0x1;
	ctrl_reg = (uint32_t*)(0x10023000 + 0x08);
	*ctrl_reg = 0x1;

	// CLINT
	// mtimecmp
	ctrl_reg = (uint32_t*)(0x02004000); // Lower-half of 64-bit value
	*ctrl_reg = 32768;
	//*ctrl_reg = 1000;
	ctrl_reg = (uint32_t*)(0x02004004); // Upper-half of 64-bit value
	*ctrl_reg = 0;
	// mtime
	ctrl_reg = (uint32_t*)(0x0200BFF8); // Lower-half of 64-bit value
	*ctrl_reg = 0;
	ctrl_reg = (uint32_t*)(0x0200BFFC); // Upper-half of 64-bit value
	*ctrl_reg = 0;

	// lfrosc
	// This has no effect on the Red-V because the lfrosc is driven by an external crystal oscillator @ 32.768khz
	//ctrl_reg = (uint32_t*)(0x10000000 + 0x70);

	*ctrl_reg = (*ctrl_reg & 0xFFE0FFC0) | 0x00000000;

	// rtc counter_start
	ctrl_reg = (uint32_t*)(0x10000000 + 0x48); // Low-Half
	*ctrl_reg = 0x00000000;
	ctrl_reg = (uint32_t*)(0x10000000 + 0x4C); // High-Half
	*ctrl_reg = 0x00000000;

	// rtccmp0
	ctrl_reg = (uint32_t*)(0x10000000 + 0x60);
	*ctrl_reg = 0xF0000000;

	__asm__ __volatile__ ("csrrw zero, mtvec, %0" : : "r" (((uint32_t)(&interrupt_handler)) & 0xFFFFFFFC));
	__asm__ __volatile__ ("csrrw zero, mie, %0" : : "r" (0x00000000));
	__asm__ __volatile__ ("csrrs zero, mstatus, %0" : : "r" (0x00000008));
	enable_timer_interrupt();

	// RTCCFG Enable
	ctrl_reg = (uint32_t*)(0x10000000 + 0x40);
	*ctrl_reg = 0x0000100F;

	//unsigned int outval;
	//outval = 0;
	//__asm__ __volatile__ ("csrrs %0, mstatus, zero" : "=r" (outval));
	//__asm__ __volatile__ ("csrrs x0, mstatus, %0" : "=r" (outval));

	// Turn ON the blue LED on the RED-V
	//*outp_reg |=  (1 << 5);

	unsigned int i;
	i = 0;

	//ctrl_reg = (uint32_t*)(0x10008000 + 0x0C);
	//ctrl_reg = (uint32_t*)(0x0200BFF8); // Lower-half of 64-bit value
	//ctrl_reg = (uint32_t*)(0x10000000 + 0x48); // Low-Half

	unsigned char shift_regs[2];
	unsigned int shift_reg_len;
	shift_reg_len = 16;
	shift_regs[0] = 0b01010000;
	shift_regs[1] = 0b00000000;

	write("TraceA\n");
	//write_shift_regs(outp_reg, shift_regs, shift_reg_len);

	//high(outp_reg, s_in);
	//high(outp_reg, srck);
	//high(outp_reg, 4);
	//high(outp_reg, 3);
	//high(outp_reg, 2);
	//high(outp_reg, 1);
	//high(outp_reg, 0);
	//write("HereB\n");
	//pause();
	//pause();
	//pause();
	//high(outp_reg, 22);
	//high(outp_reg, 4);
	high(outp_reg, 5);
	//pause();
	//pause();
	//pause();
	//write("HereA\n");
	//write("TraceB\n");

	//goto break_end;

	//pause();
	low(outp_reg, rck);
	pause();

	loop_0:
	high(outp_reg, s_in);
	pause();
	high(outp_reg, srck);
	pause();
	low(outp_reg, srck);
	pause();

	low(outp_reg, s_in);
	pause();
	high(outp_reg, srck);
	pause();
	low(outp_reg, srck);
	pause();

	high(outp_reg, s_in);
	pause();
	high(outp_reg, srck);
	pause();
	low(outp_reg, srck);
	pause();

	low(outp_reg, s_in);
	pause();
	//high(outp_reg, srck);
	//pause();
	//low(outp_reg, srck);
	//pause();

	high(outp_reg, rck);
	pause();
	low(outp_reg, rck);
	pause();

	write("TraceB\n");

	/*
	i = 0;
	while (i < 0) {
		char tmp_str[50];
		tmp_str[0] = '0' + i;
		tmp_str[1] = 0;
		write("Hello, World! ");
		write(tmp_str);
		//write(" ");
		//outval = *ctrl_reg;
		//itoa((outval >> 16) & 0xFFFF, tmp_str, 50, 16, 4);
		//write(tmp_str);
		////itoa((outval >>  0) & 0xFFFF, tmp_str, 50, 10, 4);
		//write(tmp_str);
		write("\n");
		cpu_wait();
		enable_timer_interrupt();
		// Toggle the blue LED on the RED-V
		*outp_reg = (*outp_reg & (1 << 5)) ^ (1 << 5);
		i++;
	}
	*/

	break_end:

	disable_timer_interrupt();

	// RTCCFG Disable
	ctrl_reg = (uint32_t*)(0x10000000 + 0x40);
	*ctrl_reg = 0x00000000;

	cpu_wait();

	return 0;
}
