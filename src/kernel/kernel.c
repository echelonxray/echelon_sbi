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

uintRL_t hart_context_count;
CPU_Context* hart_contexts;
__thread uintRL_t mhartid;

void kinit() {
	kallocinit(&KHEAP_START, &KHEAP_START + 0x8000);
	kalloc_pageinit(&KHEAP_START + 0x8000, &KHEAP_START + 0x8000 + 0x10000000);
	
	uintRL_t stack_size = 0x1000;
	// Note:
	//   &THI_START == &THI_tdata_START
	//   &THI_END == &THI_tbss_END
	//   The linker script should set them to be same.
	uintRL_t tls_size = (uintRL_t)(&THI_END) - (uintRL_t)(&THI_START);
	uintRL_t tls_init_size = (uintRL_t)(&THI_tdata_END) - (uintRL_t)(&THI_tdata_START);
	uintRL_t tls_null_size = (uintRL_t)(&THI_tbss_END) - (uintRL_t)(&THI_tbss_START);
	uintRL_t tls_null_offt = (uintRL_t)(&THI_tbss_START) - (uintRL_t)(&THI_START);
	hart_context_count = 5;
	hart_contexts = kmalloc(hart_context_count * sizeof(CPU_Context));
	void* ptr;
	
	// Setup this hart's (Hart 0) context struct
	ptr = kmalloc(tls_size);
	memcpy(ptr, &THI_tdata_START, tls_init_size);
	memset(ptr + tls_null_offt, 0, tls_null_size);
	//hart_contexts[0].mhart_tp = ptr;
	//hart_contexts[0].mhart_sp = &KISTACK_TOP;
	//hart_contexts[0].mhartid = 0;
	//hart_contexts[0].mcontext = kmalloc(sizeof(CPU_Context));
	hart_contexts[0].context_id = 0;
	hart_contexts[0].execution_mode = 3;
	hart_contexts[0].reserved_0 = 0;
	for (uintRL_t j = 0; j < 32; j++) {
		hart_contexts[0].regs[j] = 0;
	}
	hart_contexts[0].regs[REG_TP] = (uintRL_t)ptr;
	hart_contexts[0].regs[REG_SP] = (uintRL_t)&KISTACK_TOP;
	// Now that we actually have TLS memory allocated and setup for this
	// hart(M-Mode thread), set the Thread Pointer to it so that we can
	// actually use it.
	__asm__ __volatile__ ("mv tp, %0" : : "r" (ptr) : "memory");
	mhartid = 0;
	
	/*
	typedef struct {
		uintRL_t context_id;
		uintRL_t execution_mode;
		uintRL_t entry_address; // Absolute Address
		uintRL_t regs[32];
	} CPU_Context;
	*/
	
	// Setup the other hart's context structs
	for (uintRL_t i = 1; i < 5; i++) {
		ptr = kmalloc(tls_size);
		memcpy(ptr, &THI_tdata_START, tls_init_size);
		memset(ptr + tls_null_offt, 0, tls_null_size);
		//hart_contexts[i].mhart_tp = ptr;
		//hart_contexts[i].mhart_sp = kmalloc_stack(stack_size);
		//hart_contexts[i].mhartid = i;
		//hart_contexts[i].mcontext = kmalloc(sizeof(CPU_Context));
		hart_contexts[i].context_id = i;
		hart_contexts[i].execution_mode = 3;
		hart_contexts[i].reserved_0 = 0;
		for (uintRL_t j = 0; j < 32; j++) {
			hart_contexts[i].regs[j] = 0;
		}
		hart_contexts[i].regs[REG_TP] = (uintRL_t)ptr;
		hart_contexts[i].regs[REG_SP] = (uintRL_t)kmalloc_stack(stack_size);
		// The special registers (sp and tp) in these harts will be set up
		// by the one-time interrupt handler that they run when we interrupt
		// them from their current spinning/halted state.
	}
	
	return;
}

void wait_by_spin() {
	for (volatile unsigned int i = 0; i < 200000000; i++) {}
	return;
}

void kmain() {
	volatile uint32_t* ctrl_reg;

	// Enable TX on UART0
	ctrl_reg = (uint32_t*)(UART0_BASE + UART_TXCTRL);
	*ctrl_reg = 0x1;

	DEBUG_print("Hello, World!\n");
	DEBUG_print("\n");
	
	ctrl_reg = (uint32_t*)CLINT_BASE;
	wait_by_spin();
	
	DEBUG_print("Sending H1 Wake Up...");
	ctrl_reg[1] = 0x1;
	DEBUG_print("Sent\n");
	wait_by_spin();

	DEBUG_print("Sending H2 Wake Up...");
	ctrl_reg[2] = 0x1;
	DEBUG_print("Sent\n");
	wait_by_spin();

	DEBUG_print("Sending H3 Wake Up...");
	ctrl_reg[3] = 0x1;
	DEBUG_print("Sent\n");
	wait_by_spin();

	DEBUG_print("Sending H4 Wake Up...");
	ctrl_reg[4] = 0x1;
	DEBUG_print("Sent\n");
	wait_by_spin();

	DEBUG_print("\n");
	DEBUG_print("Bye, World!\n");

	return;
}
