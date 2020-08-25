#ifndef _insertion_kernel_globals_h
#define _insertion_kernel_globals_h

#include "./../inc/types.h"

struct cpu_context {
	uintRL_t gen_regs[32];
#ifdef __riscv_flen
	// Handle Floating Point Registers
#endif
};

extern struct cpu_context* cpu_context_ptr;

#endif
