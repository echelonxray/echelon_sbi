// This is a stub
 /*
void write_shift_regs(unsigned char* regs, unsigned int len) {
	unsigned int pin_shft_serial_out;
	unsigned int pin_shft_serial_clk;
	unsigned int pin_shft_update_out;
	pin_shft_serial_out = 4;
	pin_shft_serial_clk = 3;
	pin_shft_update_out = 2;
	
	// Work backwards through the bits since we are 
	// putting them into a shift register.
	unsigned char* working_byte_ptr;
	unsigned int bit_of_byte;
	while (len > 0) {
		len--;
		working_byte_ptr = regs + (len / 8);
		bit_of_byte = len % 8;
		if ((*working_byte_ptr) & (1 << bit_of_byte)) {
			HIGH(pin_shft_serial_out);
		} else {
			LOW(pin_shft_serial_out);
		}
		pause();
		
		HIGH(pin_shft_serial_clk);
		pause();
		
		LOW(pin_shft_serial_clk);
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
	regs[1] = 0b00000001;
	write_shift_regs(regs, 11);
	regs[0] = 0b00000000;
	regs[1] = 0b00000000;
	write_shift_regs(regs, 11);
	return;
}
void write_data(unsigned char data) {
	unsigned char regs[2];
	regs[0] = data;
	regs[1] = 0b00000101;
	write_shift_regs(regs, 11);
	regs[0] = 0b00000000;
	regs[1] = 0b00000000;
	write_shift_regs(regs, 11);
	return;
}
*/

#include "./init.h"
#include "./../../kernel/inc/memmap.h"

signed int init_main(unsigned int argc, char* argv[], char* envp[]) {
	uart_write("Part #1!\n", UART0_BASE);
	__asm__ __volatile__ ("ecall");
	uart_write("Part #2!\n", UART0_BASE);
	while (1) {
		//write(1, "Hello, world!\n", 14);
	}
	return 0;
}
