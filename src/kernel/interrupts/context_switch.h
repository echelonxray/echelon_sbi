#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uintRL_t context_id;
	uintRL_t execution_mode;
	uintRL_t entry_address; // Absolute Address
	uintRL_t regs[32];
} CPU_Context;

void switch_context(CPU_Context* cpu_context);
void interrupt_entry_handler();
void interrupt_c_handler(CPU_Context* cpu_context, uintRL_t is_interrupt, uintRL_t cause_value);

#endif
