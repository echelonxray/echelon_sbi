#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

typedef struct {
	uint32_t context_id;
	uint32_t status_vals;
	uint32_t program_memory;
	uintRL_t regs[32];
} CPU_Context;

void switch_context(CPU_Context* cpu_context);

#endif
