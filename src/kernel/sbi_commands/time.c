#include "time.h"
#include "./../inc/memmap.h"

extern __thread uintRL_t mhartid;

struct sbiret sbi_set_timer(uint64_t stime_value) {
	uint64_t* mtimecmps = (void*)(CLINT_BASE + CLINT_MTIMECMPS);
	uint64_t* mtime = (void*)(CLINT_BASE + CLINT_MTIME);
	__asm__ __volatile__ ("csrc mip, %0" : : "r" (0x20));
	mtimecmps[mhartid] = *mtime + stime_value;
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
