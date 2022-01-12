#include "time.h"
#include "./../inc/memmap.h"

extern __thread uintRL_t mhartid;

struct sbiret sbi_set_timer(uint64_t stime_value) {
	__asm__ __volatile__ ("csrc mip, %0" : : "r" (0x20));
	__asm__ __volatile__ ("csrs mie, %0" : : "r" (0x80));
	
#ifdef MM_JSEMU_0000
	uint32_t mtime;
	uint32_t mtimeh;
	mtimeh = stime_value >> 32;
	mtimeh = stime_value >>  0;
	
	volatile uint32_t* mtimecmp = (void*)(CLINT_BASE + CLINT_MTIMECMP);
	volatile uint32_t* mtimecmphi = (void*)(CLINT_BASE + CLINT_MTIMECMPHI);
	mtimecmphi[mhartid] = 0xFFFFFFFF;
	mtimecmp[mhartid]   = mtime;
	mtimecmphi[mhartid] = mtimeh;
#endif
	
#ifdef MM_FU540_C000
	uint64_t* mtimecmp = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
	mtimecmp[mhartid] = stime_value;
#endif
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
