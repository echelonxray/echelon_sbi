#include "./../inc/memmap.h"

#if   __riscv_xlen == 128
	#error context_switch_asm_128.s is not implemented.  128-bit port required.
#elif __riscv_xlen == 64
	#include "./context_switch_asm_64.s"
#else
	#include "./context_switch_asm_32.s"
#endif
