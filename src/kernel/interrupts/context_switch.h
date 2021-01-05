#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uint32_t context_id;
	uint32_t status_vals;
	uint32_t RESERVED;
	uintRL_t regs[32];
} CPU_Context;

uint32_t load_context_from_function_ptr(void* entry_pt);
void switch_context(CPU_Context* cpu_context);

#endif
