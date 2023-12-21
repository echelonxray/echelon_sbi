#include "time.h"
#include <inc/memmap.h>
#include <inc/csr.h>

extern __thread uintRL_t mhartid;

struct sbiret sbi_set_timer(uint64_t stime_value) {
	CSRI_BITCLR(CSR_MIE, 0x80);
	CSRI_BITCLR(CSR_MIP, 0x20);
	
#ifdef MM_CUSTOM_EMU
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

#ifdef MM_QEMU_VIRT

#if   __riscv_xlen == 64
	volatile uint64_t* mtimecmp = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
	mtimecmp[mhartid] = stime_value;
#elif __riscv_xlen == 32
	uint32_t mtime;
	uint32_t mtimeh;
	mtimeh = (uint32_t)((stime_value >> 32) & 0xFFFFFFFF);
	mtime  = (uint32_t)((stime_value >>  0) & 0xFFFFFFFF);
	
	volatile uint32_t* mtimecmp   = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
	volatile uint32_t* mtimecmphi = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS + 0x4);
	mtimecmphi[mhartid * 2] = (sint32_t)-1;
	mtimecmp[mhartid * 2] = mtime;
	mtimecmphi[mhartid * 2] = mtimeh;
#endif

#endif
	
	CSRI_BITSET(CSR_MIE, 0x80);
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
