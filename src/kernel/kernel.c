#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./inc/memmap.h"
#include "./inc/gpio_oper.h"
#include "./drivers/uart.h"
#include "./interrupts/context_switch.h"
#include "./memalloc.h"
#include "./thread_locking.h"
#include "./globals.h"
#include "./debug.h"

ksemaphore_t* hart_command_que_locks;
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
	//kalloc_pageinit(&KHEAP_START + 0x8000, &KHEAP_START + 0x8000 + 0x10000000);
	
	hart_commands = kmalloc(TOTAL_HART_COUNT * sizeof(Hart_Command));
	hart_command_que_locks = kmalloc(TOTAL_HART_COUNT * sizeof(ksemaphore_t));
	for (uintRL_t i = 0; i < TOTAL_HART_COUNT; i++) {
		hart_commands[i].command = 0;
		hart_commands[i].param0 = 0;
		hart_commands[i].param1 = 0;
		hart_commands[i].param2 = 0;
		hart_commands[i].param3 = 0;
		hart_commands[i].param4 = 0;
		hart_commands[i].param5 = 0;
		ksem_init(hart_command_que_locks + i);
	}
	
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
	
	// Setup this hart's (Hart 0) context struct
	void* ptr;
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
	
	// Start and wait for other harts to initialize
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
		clint_hart_msip_ctls[i] = 0x1;
	}
	for (uintRL_t i = 1; i < TOTAL_HART_COUNT; i++) {
		while (clint_hart_msip_ctls[i]) {}
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
	
	clear_hart_context(hart_contexts + TOTAL_HART_COUNT + 0);
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_PC] = 0x20000000;
	hart_contexts[TOTAL_HART_COUNT + 0].regs[REG_SP] = (uintRL_t)kmalloc_stack(STACK_SIZE);
	hart_contexts[TOTAL_HART_COUNT + 0].context_id = TOTAL_HART_COUNT + 0;
	hart_contexts[TOTAL_HART_COUNT + 0].execution_mode = EM_S;
	
	Hart_Command command;
	
	command.command = HARTCMD_SETEXCEPTIONDELEGATION;
	command.param0 = 1 << 8;
	send_hart_command_blk(1, &command);
	
	command.command = HARTCMD_SWITCHCONTEXT;
	command.param0 = (uintRL_t)(hart_contexts + TOTAL_HART_COUNT + 0);
	send_hart_command_blk(1, &command);
	
	wait_by_spin();
	
	DEBUG_print("\n");
	DEBUG_print("Bye, World!\n");
	
	return;
}
