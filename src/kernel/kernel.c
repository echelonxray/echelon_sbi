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
volatile CPU_Context* hart_contexts;
volatile Hart_Command* hart_commands;

__thread uintRL_t mhartid;

#define TOTAL_HART_COUNT 5
#define USE_HART_COUNT 4
#define STACK_SIZE 0x1000

void clear_hart_context(volatile CPU_Context* hart_context) {
	hart_context->context_id = 0;
	hart_context->execution_mode = 0;
	hart_context->reserved_0 = 0;
	for (uintRL_t j = 0; j < 32; j++) {
		hart_context->regs[j] = 0;
	}
	return;
}

void kinit() {
	kallocinit(&KHEAP_START, &KHEAP_START + 0x8000);
	kalloc_pageinit(&KHEAP_START + 0x8000, &KHEAP_START + 0x8000 + 0x10000000);
	
	// Note:
	//   &THI_START == &THI_tdata_START
	//   &THI_END == &THI_tbss_END
	//   The linker script should set them to be same.
	uintRL_t tls_size = (uintRL_t)(&THI_END) - (uintRL_t)(&THI_START);
	uintRL_t tls_init_size = (uintRL_t)(&THI_tdata_END) - (uintRL_t)(&THI_tdata_START);
	uintRL_t tls_null_size = (uintRL_t)(&THI_tbss_END) - (uintRL_t)(&THI_tbss_START);
	uintRL_t tls_null_offt = (uintRL_t)(&THI_tbss_START) - (uintRL_t)(&THI_START);
	hart_context_count = (TOTAL_HART_COUNT + (USE_HART_COUNT * 1));
	hart_contexts = kmalloc(hart_context_count * sizeof(CPU_Context));
	hart_commands = kmalloc(TOTAL_HART_COUNT * sizeof(Hart_Command));
	void* ptr;
	
	// Setup this hart's (Hart 0) context struct
	ptr = kmalloc(tls_size);
	memcpy(ptr, &THI_tdata_START, tls_init_size);
	memset(ptr + tls_null_offt, 0, tls_null_size);
	clear_hart_context(hart_contexts + 0);
	hart_contexts[0].context_id = 0;
	hart_contexts[0].execution_mode = EM_M;
	hart_contexts[0].regs[REG_TP] = (uintRL_t)ptr;
	hart_contexts[0].regs[REG_SP] = (uintRL_t)&KISTACK_TOP;
	// Now that we actually have TLS memory allocated and setup for this
	// hart(M-Mode thread), set the Thread Pointer to it so that we can
	// actually use it.
	__asm__ __volatile__ ("mv tp, %0" : : "r" (ptr) : "memory");
	mhartid = 0;
	
	// Setup the other hart's context structs
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
		ptr = kmalloc(tls_size);
		memcpy(ptr, &THI_tdata_START, tls_init_size);
		memset(ptr + tls_null_offt, 0, tls_null_size);
		clear_hart_context(hart_contexts + i);
		hart_contexts[i].context_id = i;
		hart_contexts[i].execution_mode = EM_M;
		hart_contexts[i].regs[REG_TP] = (uintRL_t)ptr;
		hart_contexts[i].regs[REG_SP] = (uintRL_t)kmalloc_stack(STACK_SIZE);
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
	
	volatile uint8_t* memory_read = (uint8_t*)0x20000000ul;
	char buf[20];
	DEBUG_print("QSPI Read 0x");
	itoa(memory_read[0], buf, 20, -16, -2);
	DEBUG_print(buf);
	itoa(memory_read[1], buf, 20, -16, -2);
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(memory_read[2], buf, 20, -16, -2);
	DEBUG_print(buf);
	itoa(memory_read[3], buf, 20, -16, -2);
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(memory_read[4], buf, 20, -16, -2);
	DEBUG_print(buf);
	itoa(memory_read[5], buf, 20, -16, -2);
	DEBUG_print(buf);
	DEBUG_print("_");
	itoa(memory_read[6], buf, 20, -16, -2);
	DEBUG_print(buf);
	itoa(memory_read[7], buf, 20, -16, -2);
	DEBUG_print(buf);
	DEBUG_print("\n");
	DEBUG_print("\n");
	
	ctrl_reg = (uint32_t*)CLINT_BASE;
	//wait_by_spin();
	
	DEBUG_print("Sending H1 Wake Up...");
	ctrl_reg[1] = 0x1;
	//DEBUG_print("Sent\n");
	while (ctrl_reg[1]) {}
	//wait_by_spin();

	DEBUG_print("Sending H2 Wake Up...");
	ctrl_reg[2] = 0x1;
	//DEBUG_print("Sent\n");
	while (ctrl_reg[2]) {}
	//wait_by_spin();

	DEBUG_print("Sending H3 Wake Up...");
	ctrl_reg[3] = 0x1;
	//DEBUG_print("Sent\n");
	while (ctrl_reg[3]) {}
	//wait_by_spin();

	DEBUG_print("Sending H4 Wake Up...");
	ctrl_reg[4] = 0x1;
	//DEBUG_print("Sent\n");
	while (ctrl_reg[4]) {}
	//wait_by_spin();

	DEBUG_print("\n");
	//wait_by_spin();

	/*
	struct hart_command {
		uintRL_t command;
		uintRL_t param0;
		uintRL_t param1;
		uintRL_t param2;
		uintRL_t param3;
		uintRL_t param4;
		uintRL_t param5;
	};
	*/
	clear_hart_context(hart_contexts + TOTAL_HART_COUNT + 0);
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_PC] = 0x20000000;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_SP] = (uintRL_t)kmalloc_stack(STACK_SIZE);
	hart_contexts[TOTAL_HART_COUNT + 0].context_id = TOTAL_HART_COUNT + 0;
	hart_contexts[TOTAL_HART_COUNT + 0].execution_mode = EM_S;
	hart_commands[1].command = HARTCMD_SWITCHCONTEXT;
	hart_commands[1].param0 = (uintRL_t)(hart_contexts + TOTAL_HART_COUNT + 0);
	ctrl_reg[1] = 0x1;
	while (ctrl_reg[1]) {}
	wait_by_spin();

	DEBUG_print("\n");
	DEBUG_print("Bye, World!\n");

	return;
}
