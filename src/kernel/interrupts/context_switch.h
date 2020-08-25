#ifndef _insertion_kernel_interrupts_context_switch_h
#define _insertion_kernel_interrupts_context_switch_h

#include "./../../inc/types.h"

uint32_t load_context_from_function_ptr(void* entry_pt);
void switch_context(uint32_t context_id);

#endif
