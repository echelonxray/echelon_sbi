#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./../progs/init/init.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./idle/idle_loop.h"
#include "./drivers/uart.h"
#include "./interrupts/base.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./globals.h"
#include "./debug.h"

#define rck 2
#define srck 3
#define s_in 4

struct __attribute__ ((packed)) cpio_format_bin {
	unsigned short c_magic;
	unsigned short c_dev;
	unsigned short c_ino;
	unsigned short c_mode;
	unsigned short c_uid;
	unsigned short c_gid;
	unsigned short c_nlink;
	unsigned short c_rdev;
	unsigned short c_mtime[2];
	unsigned short c_namesize;
	unsigned short c_filesize[2];
};

/*
void kwrite(char* str) {
	uart_write(str, UART0_BASE);
	return;
}
*/

signed int kmain(unsigned int argc, char* argv[], char* envp[]) {
	// START: CPU Init
	volatile uint32_t* urat_reg;
	volatile uint32_t* ctrl_reg;
	volatile uint32_t* prci_reg;
	// Setup the Clock to 256MHz by setting up the PLL Frequency Multipiers and Dividers
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_HFROSCCFG);
	*prci_reg |= (1 << 30);
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_PLLCFG);
	*prci_reg |= (1 << 16) | (1 << 17) | (1 << 18);
	// The input is 16MHz from the external crystal oscillator HFXOSC on the Redboard Red-V
	// pllr = 1 Step #1: Divide by [pllr + 1] = 2 [bitshift by 0 to locate in PLLCFG register]
	// pllf = 31 Step #2: Multiply by [2 * (pllf + 1)] = 64 [bitshift by 4 to locate in PLLCFG register]
	// pllq = 1 Step #3: Divide by [2 ^ pllq] = 2 [bitshift by 10 to locate in PLLCFG register]
	// Final Clock Frequency: 256MHz = 16MHz / 2 * 64 / 2
	*prci_reg = (*prci_reg & 0x7FFFF000) | (1 << 0) | (31 << 4) | (1 << 10);
	*prci_reg &= ~(1 << 18);
	prci_reg = (uint32_t*)(PRCI_BASE + PRCI_HFROSCCFG);
	*prci_reg &= ~(1 << 30);
	
	// Set UART frequency divider to attain a ~115200 Baud Rate
	// Clock frequency 256MHz / (2221 + 1) = ~115200 Baud
	urat_reg = (uint32_t*)(UART0_BASE + UART_DIV);
	*urat_reg = 2221;
	urat_reg = (uint32_t*)(UART1_BASE + UART_DIV);
	*urat_reg = 2221;
	
	// Disable UART Watermark Interrupts
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_IE);
	*ctrl_reg = 0x0;
	ctrl_reg = (uint32_t*)(UART1_BASE + UART_IE);
	*ctrl_reg = 0x0;
	
	// Set GPIO Pins 2, 3, 4, and 5 to GPIO mode, Output mode, and Turn Them Off
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
	
	// Set HW I/O UART0 functionality through the GPIO Pins (IOF) [Mode Select]
	IOF_SEL(16, 1);
	IOF_SEL(17, 1);
	
	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;
	
	// CLINT
	// mtimecmp
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_MTIMECMP_LO); // Lower-half of 64-bit value
	*ctrl_reg = 32768 / 2;
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
	//ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTLO); // Low-Half
	//*ctrl_reg = 0x00000000;
	//ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCOUNTHI); // High-Half
	//*ctrl_reg = 0x00000000;
	
	// rtccmp0
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCMP0);
	*ctrl_reg = 0xF0000000;
	
	__asm__ __volatile__ ("csrrw zero, mtvec, %0" : : "r" (&interrupt_handler));
	__asm__ __volatile__ ("csrrw zero, mie, %0" : : "r" (0x00000000));
	__asm__ __volatile__ ("csrrc zero, mstatus, %0" : : "r" (0x00000008));
	
	// RTCCFG Enable
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCFG);
	*ctrl_reg = 0x0000100F;
	// END: CPU Init
	
	// START: Kernel Init
	kallocinit((void*)&KHEAP_START, (void*)0x80003FFF);
	kernel_stack_base = (uintRL_t)kmalloc(0x1000);
	kernel_stack_top = kernel_stack_base + 0x1000;
	
	__asm__ __volatile__ ("csrrw zero, pmpcfg0, %0" : : "r" (0x00080F08));
	//__asm__ __volatile__ ("csrrw zero, pmpcfg1, %0" : : "r" (0x00000000));
	
	//__asm__ __volatile__ ("csrrw zero, pmpaddr0, %0" : : "r" (0x08000000));
	//__asm__ __volatile__ ("csrrw zero, pmpaddr1, %0" : : "r" (0x10000000));
	//__asm__ __volatile__ ("csrrw zero, pmpaddr4, %0" : : "r" (0x40000000));
	// END: Kernel Init
	
	// START: Userspace Init
	CPU_Context* test_context_ptr;
	test_context_ptr = kmalloc(sizeof(CPU_Context));
	memset(test_context_ptr, 0, sizeof(CPU_Context));
	test_context_ptr->context_id = 1;
	test_context_ptr->status_vals = (0 << 0);
	
	void* cpio_archive = &USERSPACE_ARCHIVE_START;
	void* elf_data = 0;
	uint32_t elf_size = 0;
	while (1) {
		struct cpio_format_bin CpioHeader;
		memcpy(&CpioHeader, cpio_archive, sizeof(struct cpio_format_bin));
		cpio_archive += sizeof(struct cpio_format_bin);
		if (CpioHeader.c_magic != 0x71C7) {
			break;
		}
		
		if (CpioHeader.c_namesize != 0) {
			if (CpioHeader.c_mode == 0) {
				if (strncmp("TRAILER!!!", cpio_archive, 11) == 0) {
					break;
				}
			}
			if ((CpioHeader.c_mode & 0170000) == 0100000) {
				if (strncmp("init.out", cpio_archive, 9) == 0) {
					elf_data = cpio_archive + CpioHeader.c_namesize + (CpioHeader.c_namesize & 1);
					elf_size = (CpioHeader.c_filesize[0] << 16) | (CpioHeader.c_filesize[1] << 0);
					break;
				}
			}
		}
		
		cpio_archive += CpioHeader.c_namesize + (CpioHeader.c_namesize & 1);
		elf_size = (CpioHeader.c_filesize[0] << 16) | (CpioHeader.c_filesize[1] << 0);
		elf_size += elf_size & 1;
		cpio_archive += elf_size;
	}
	if (elf_data == 0) {
		DEBUG_print("Failed locate Init program!\n");
		idle_loop();
	}
	
	//test_context_ptr->regs[0] = &init_main;
	//test_context_ptr->regs[0] = 0;
	void* prog_mem_base;
	void* prog_mem_top;
	uint32_t ret_code;
	ret_code = load_context_from_elf_data(elf_data, elf_size, test_context_ptr, &prog_mem_base, &prog_mem_top);
	if (ret_code != 0) {
		DEBUG_print("ret_code: ");
		char buff[20];
		memset(buff, 0, 20);
		itoa(ret_code, buff, 20, 10, 0);
		DEBUG_print(buff);
		DEBUG_print("\n");
		DEBUG_print("Failed to read Init program!\n");
		idle_loop();
	}
	//prog_stack_base = (uintRL_t)kmalloc(0x1000);
	//prog_stack_top = (uintRL_t)(prog_stack_base + 0x1000);
	//test_context_ptr->regs[2] = prog_stack_top;
	__asm__ __volatile__ ("csrrw zero, pmpaddr0, %0" : : "r" (((uintRL_t)prog_mem_base) >> 2));
	__asm__ __volatile__ ("csrrw zero, pmpaddr1, %0" : : "r" (((uintRL_t)prog_mem_top) >> 2));
	//__asm__ __volatile__ ("csrrs zero, mstatus, %0" : : "r" (0x00001880));
	//	Start Userspace
	switch_context(test_context_ptr);
	// END: Userspace Init
	
	//HIGH(5);
	
	// RTCCFG Disable
	ctrl_reg = (uint32_t*)(AON_BASE + AON_RTCCFG);
	*ctrl_reg = 0x00000000;
	
	DEBUG_print("TraceY\n");
	CPU_WAIT();
	DEBUG_print("TraceZ\n");
	
	return 0;
}
