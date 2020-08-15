#include "./inc/types.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"

#define rck 2
#define srck 3
#define s_in 4

void interrupt_handler();

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

void write(char* str) {
	if (str == 0) {
		return;
	}
	unsigned int i;
	i = 0;
	while (str[i] != 0) {
		uart_putc(str[i], (void*)UART0_BASE);
		//uart_putc(str[i], (void*)UART1_BASE);
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
			DISABLE_TIMER_INTERRUPT();

			// mtime
			ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_LO); // Lower-half of 64-bit value
			*ctrl_reg = 0;
			ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_HI); // Upper-half of 64-bit value
			*ctrl_reg = 0;

			// rtc counter_start
			ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTLO); // Low-Half
			*ctrl_reg = 0;
			ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTHI); // High-Half
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

void pause() {
	CPU_WAIT();
	ENABLE_TIMER_INTERRUPT();
}

void write_shift_regs(unsigned char* regs, unsigned int len) {
	unsigned int pin_shft_serial_out;
	unsigned int pin_shft_serial_clk;
	unsigned int pin_shft_update_out;
	pin_shft_serial_out = 4;
	pin_shft_serial_clk = 3;
	pin_shft_update_out = 2;

	unsigned int i;
	i = 0;
	while (i < len) {
		if ((*regs) & (1 << i)) {
			HIGH(pin_shft_serial_out);
		} else {
			LOW(pin_shft_serial_out);
		}
		pause();
		
		HIGH(pin_shft_serial_clk);
		pause();
		
		LOW(pin_shft_serial_clk);
		if (i == 7) {
			i = 0;
			len -= 8;
		} else {
			i++;
		}
	}
	
	HIGH(pin_shft_update_out);
	pause();
	LOW(pin_shft_update_out);
	//pause();

	return;
}

void write_command(unsigned char command) {
	unsigned char regs[2];
	regs[0] = command;
	regs[1] = 0b00000000;
	return;
}
void write_data(unsigned char data) {
	unsigned char regs[2];
	regs[0] = data;
	regs[1] = 0b00000000;
	return;
}

signed int main(unsigned int argc, char* argv[], char* envp[]) {
	volatile uint32_t* urat_reg;
	volatile uint32_t* ctrl_reg;
	//volatile uint32_t* outp_reg;
	volatile uint32_t* prci_reg;

	// Setup the Clock
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_HFROSCCFG);
	*prci_reg |= (1 << 30);
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_PLLCFG);
	*prci_reg |= (1 << 16) | (1 << 17) | (1 << 18);
	*prci_reg = (*prci_reg & 0x7FFFF000) | 0x000005F1;
	*prci_reg &= ~(1 << 18);
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_HFROSCCFG);
	*prci_reg &= ~(1 << 30);

	// Set Div to 16MHz / (138 + 1) = ~115200 Baud
	urat_reg = (uint32_t*)(UART0_BASE + UART_DIV);
	*urat_reg = 2221;
	urat_reg = (uint32_t*)(UART1_BASE + UART_DIV);
	*urat_reg = 2221;

	// Disable UART Watermark Interrupts
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_IE);
	*ctrl_reg = 0x0;
	ctrl_reg = (uint32_t*)(UART1_BASE + UART_IE);
	*ctrl_reg = 0x0;

	// Set GPIO Pins 0, 1, and 5 to GPIO mode, Output mode, and Turn Them Off
	IOF_SEL(5, 0);
	IOF_SEL(4, 0);
	IOF_SEL(3, 0);
	IOF_SEL(2, 0);
	OUTPUT(5);
	OUTPUT(4);
	OUTPUT(3);
	OUTPUT(2);
	LOW(5);
	LOW(4);
	LOW(3);
	LOW(2);

	// Set HW I/O UART0 functionality through the GPIO Pins (IOF)
	IOF_SEL(16, 1);
	IOF_SEL(17, 1);

	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;

	// CLINT
	// mtimecmp
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIMECMP_LO); // Lower-half of 64-bit value
	*ctrl_reg = 32768;
	//*ctrl_reg = 1000;
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIMECMP_HI); // Upper-half of 64-bit value
	*ctrl_reg = 0;
	// mtime
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_LO); // Lower-half of 64-bit value
	*ctrl_reg = 0;
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIME_HI); // Upper-half of 64-bit value
	*ctrl_reg = 0;

	// lfrosc
	// This has no effect on the Red-V because the lfrosc is driven by an external crystal oscillator @ 32.768khz
	//ctrl_reg = (uint32_t*)(AON_BASE + AON_LFROSCCFG);

	// rtc counter_start
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTLO); // Low-Half
	*ctrl_reg = 0x00000000;
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTHI); // High-Half
	*ctrl_reg = 0x00000000;

	// rtccmp0
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCMP0);
	*ctrl_reg = 0xF0000000;

	__asm__ __volatile__ ("csrrw zero, mtvec, %0" : : "r" (&interrupt_handler));
	__asm__ __volatile__ ("csrrw zero, mie, %0" : : "r" (0x00000000));
	__asm__ __volatile__ ("csrrs zero, mstatus, %0" : : "r" (0x00000008));
	ENABLE_TIMER_INTERRUPT();

	// RTCCFG Enable
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCFG);
	*ctrl_reg = 0x0000100F;
	
	HIGH(5);
	
	LOW(s_in);
	LOW(srck);
	LOW(rck);
	pause();
	
	HIGH(s_in);
	pause();
	HIGH(srck);
	pause();
	LOW(srck);
	pause();
	
	LOW(s_in);
	pause();
	HIGH(srck);
	pause();
	LOW(srck);
	pause();
	
	HIGH(s_in);
	pause();
	HIGH(srck);
	pause();
	LOW(srck);
	pause();
	
	LOW(s_in);
	pause();

	HIGH(rck);
	pause();
	LOW(rck);
	pause();

	DISABLE_TIMER_INTERRUPT();

	// RTCCFG Disable
	ctrl_reg = (uint32_t*)(0x10000000 + 0x40);
	*ctrl_reg = 0x00000000;

	CPU_WAIT();

	return 0;
}
