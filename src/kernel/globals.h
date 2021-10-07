#ifndef _insertion_kernel_globals_h
#define _insertion_kernel_globals_h

#include "./../inc/types.h"

struct hart_m_context {
	uintRL_t mhartid;
	uintRL_t mhart_sp;
	uintRL_t mhart_tp;
};

extern void** KISTACK_BOTTOM;
extern void** KISTACK_TOP;
extern void** KHEAP_START;

#endif
