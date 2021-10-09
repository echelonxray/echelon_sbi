#ifndef _insertion_kernel_globals_h
#define _insertion_kernel_globals_h

#include "./../inc/types.h"

struct hart_m_context {
	uintRL_t mhartid;
	void* mhart_sp;
	void* mhart_tp;
};

// Typeless symbols from the linker script

// Stack
extern void** KISTACK_BOTTOM;
extern void** KISTACK_TOP;

// Heap
extern void** KHEAP_START;

// TLS
extern void** THI_START;
extern void** THI_END;
extern void** THI_tdata_START;
extern void** THI_tdata_END;
extern void** THI_tbss_START;
extern void** THI_tbss_END;

// Unused in C: Other
//PROGAMIMAGE_START
//INIT_DATA_PROGAMIMAGE_START
//INIT_DATA_RUNTIME_START
//INIT_DATA_RUNTIME_END
//__global_pointer$

#endif
