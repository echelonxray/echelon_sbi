#include "time.h"
#include "./../inc/memmap.h"

extern __thread uintRL_t mhartid;

struct sbiret sbi_set_timer(uint64_t stime_value) {
	__asm__ __volatile__ ("csrc mip, %0" : : "r" (0x20));
	__asm__ __volatile__ ("csrs mie, %0" : : "r" (0x80));
	
	uint32_t mtime;
	uint32_t mtimeh;
	__asm__ __volatile__ ("csrr %0, time"  : "=r" (mtime));
	__asm__ __volatile__ ("csrr %0, timeh" : "=r" (mtimeh));
	
	uint64_t ttime;
	ttime   = mtimeh;
	ttime <<= 32;
	ttime  |= mtime;
	
	ttime += stime_value;
	
	uint32_t* mtimecmp = (void*)(CLINT_BASE + CLINT_MTIMECMP);
	uint32_t* mtimecmphi = (void*)(CLINT_BASE + CLINT_MTIMECMPHI);
	mtimecmphi[mhartid] = 0xFFFFFFFF;
	mtimecmp[mhartid]   = (uint32_t)((ttime >>  0) & 0xFFFFFFFF);
	mtimecmphi[mhartid] = (uint32_t)((ttime >> 32) & 0xFFFFFFFF);
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
