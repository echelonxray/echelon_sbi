#include "time.h"
#include "./../inc/memmap.h"

extern __thread uintRL_t mhartid;

struct sbiret sbi_set_timer(uint64_t stime_value) {
	//DEBUG_print("sbi_set_timer();\n");
	
	__asm__ __volatile__ ("csrc mie, %0" : : "r" (0x80));
	__asm__ __volatile__ ("csrc mip, %0" : : "r" (0x20));
	
#ifdef MM_JSEMU_0000
	uint32_t mtime;
	uint32_t mtimeh;
	mtimeh = (uint32_t)((stime_value >> 32) & 0xFFFFFFFF);
	mtime  = (uint32_t)((stime_value >>  0) & 0xFFFFFFFF);
	
	volatile uint32_t* mtimecmp = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
	volatile uint32_t* mtimecmphi = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS + 0x4);
	mtimecmphi[mhartid * 2] = (sint32_t)-1;
	mtimecmp[mhartid * 2]   = mtime;
	mtimecmphi[mhartid * 2] = mtimeh;
#endif
	
#ifdef MM_FU540_C000
	volatile uint64_t* mtimecmp = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMP);
	mtimecmp[mhartid] = stime_value;
#endif
	
	__asm__ __volatile__ ("csrs mie, %0" : : "r" (0x80));
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
