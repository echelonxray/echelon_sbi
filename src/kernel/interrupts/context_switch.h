#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uintRL_t context_id;
	uintRL_t execution_mode;
	uintRL_t reserved_0;
	uintRL_t regs[32];
} CPU_Context;

void* hart_start_c_handler(uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value);
void interrupt_c_handler(CPU_Context* cpu_context, uintRL_t hart_context_index, uintRL_t is_interrupt, uintRL_t cause_value);
void interrupt_entry_handler();
void switch_context(CPU_Context* cpu_context);

#endif
