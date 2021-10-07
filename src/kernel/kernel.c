#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./globals.h"
#include "./debug.h"

struct hart_m_context {
	uintRL_t mhartid;
	uintRL_t mhart_sp;
	uintRL_t mhart_tp;
};

uintRL_t hart_m_context_count;
struct hart_m_context* hart_m_contexts;

void print_state() {
	static unsigned int counter = 0;
	//__asm__ __volatile__ ("fence.i");
	volatile uint32_t* state = (uint32_t*)(0x80010000u);
	char buf[50];
	DEBUG_print("Cores ");
	itoa(counter, buf, 50, -10, -1);
	DEBUG_print(buf);
	DEBUG_print(": ");
	itoa(*state, buf, 50, -2, -8);
	DEBUG_print(buf);
	DEBUG_print("\n");
	counter++;
	for (volatile unsigned int i = 0; i < 0500000000; i++) {}
	return;
}

void kmain() {
	// START: CPU Init
	volatile uint32_t* ctrl_reg;
	
	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;
	
	kallocinit((void*)KHEAP_START, (void*)KHEAP_START + 0x8000);
	
	hart_m_context_count = 4;
	hart_m_contexts = kmalloc(hart_m_context_count * sizeof(struct hart_m_context));
	hart_m_contexts[0].mhartid = 1;
	hart_m_contexts[0].mhart_sp = kmalloc(0x1000);
	hart_m_contexts[0].mhart_tp = 0;
	hart_m_contexts[1].mhartid = 2;
	hart_m_contexts[1].mhart_sp = kmalloc(0x1000);
	hart_m_contexts[1].mhart_tp = 0;
	hart_m_contexts[2].mhartid = 3;
	hart_m_contexts[2].mhart_sp = kmalloc(0x1000);
	hart_m_contexts[2].mhart_tp = 0;
	hart_m_contexts[3].mhartid = 4;
	hart_m_contexts[3].mhart_sp = kmalloc(0x1000);
	hart_m_contexts[3].mhart_tp = 0;
	
	DEBUG_print("Hello, World!\n");
	DEBUG_print("\n");
	
	print_state();
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_H0_MSIP);
	*ctrl_reg = 0x1;
	
	print_state();
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_H1_MSIP);
	*ctrl_reg = 0x1;
	
	print_state();
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_H2_MSIP);
	*ctrl_reg = 0x1;
	
	print_state();
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_H3_MSIP);
	*ctrl_reg = 0x1;
	
	print_state();
	ctrl_reg = (uint32_t*)(CLINT_BASE + CLINT_H4_MSIP);
	*ctrl_reg = 0x1;
	
	print_state();
	print_state();
	
	DEBUG_print("\n");
	DEBUG_print("Bye, World!\n");
	
	return;
}
