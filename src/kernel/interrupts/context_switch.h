#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uintRL_t context_id;
	uintRL_t execution_mode;
	uintRL_t entry_address; // Absolute Address
	uintRL_t regs[32];
} CPU_Context;

void interrupt_c_handler();
void interrupt_entry_handler();
void switch_context(CPU_Context* cpu_context);

#endif
