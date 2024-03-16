#include "time.h"
#include <inc/memmap.h>
#include <inc/csr.h>

extern __thread uintRL_t mhartid;
extern __thread uint8_t hart_using_ext_sstc;

struct sbiret sbi_set_timer(uint64_t stime_value) {

#if   __riscv_xlen == 64
	if (hart_using_ext_sstc) {
		CSRI_WRITE(CSR_STIMECMP, stime_value);
	} else {
		CSRI_BITCLR(CSR_MIE, 0x80);
		CSRI_BITCLR(CSR_MIP, 0x20);

		volatile uint64_t* mtimecmp = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
		mtimecmp[mhartid] = stime_value;
	}
#elif __riscv_xlen == 32
	uint32_t xtime;
	uint32_t xtimeh;
	xtimeh = (uint32_t)((stime_value >> 32) & 0xFFFFFFFF);
	xtime  = (uint32_t)((stime_value >>  0) & 0xFFFFFFFF);

	if (hart_using_ext_sstc) {
		CSRI_WRITE(CSR_STIMECMPH, (sint32_t)-1);
		CSRI_WRITE(CSR_STIMECMP, xtime);
		CSRI_WRITE(CSR_STIMECMPH, xtimeh);
	} else {
		volatile uint32_t* mtimecmp   = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS);
		volatile uint32_t* mtimecmphi = (void*)(((uintRL_t)CLINT_BASE) + CLINT_MTIMECMPS + 0x4);
		mtimecmphi[mhartid * 2] = (sint32_t)-1;
		mtimecmp[mhartid * 2] = xtime;
		mtimecmphi[mhartid * 2] = xtimeh;

		CSRI_BITSET(CSR_MIE, 0x80);
	}
#endif

	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
