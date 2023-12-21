#include "ipi.h"
#include <main.h>
#include <thread_locking.h>
#include <inc/memmap.h>
#include <inc/csr.h>
#include <interrupts/context_switch.h>

extern ksemaphore_t* hart_command_que_locks;
extern __thread uintRL_t mhartid;

struct sbiret sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base) {
	// Identify hearts to command
	if (hart_mask_base == (unsigned long)-1) {
		hart_mask = 0;
		for (uintRL_t i = TOTAL_HART_COUNT - USE_HART_COUNT; i < TOTAL_HART_COUNT; i++) {
			hart_mask |= (1ul << i);
		}
		hart_mask_base = 0;
	}
	
	// Check for invalid harts
	unsigned long tmp_hart_mask;
	tmp_hart_mask = hart_mask;
	for (uintRL_t i = 0; i < __riscv_xlen; i++) {
		if (tmp_hart_mask & 0x1) {
			uintRL_t hartid = hart_mask_base + i;
			if (is_valid_hartid(hartid) == 0) {
				DEBUG_print("SBI FAILURE: SEND_IPI\n");
				struct sbiret retval;
				retval.value = 0;
				retval.error = SBI_ERR_INVALID_PARAM;
				return retval;
			}
		}
		tmp_hart_mask >>= 1;
	}
	
	// Do the action on the harts
	tmp_hart_mask = hart_mask;
	for (uintRL_t i = 0; i < __riscv_xlen; i++) {
		if (tmp_hart_mask & 0x1) {
			uintRL_t hartid = hart_mask_base + i;
			if (hartid == mhartid) {
				CSRI_BITSET(CSR_MIP, 0x2);
			} else {
				Hart_Command command;
				command.command = HARTCMD_SMODE_SOFTINT;
				command.param0 = hartid;
				send_hart_command_que(hartid, &command);
				//send_hart_command_lck(i, &command);
			}
		}
		tmp_hart_mask >>= 1;
	}
	
	/*
	volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
	for (uintRL_t i = 0; i < hart_count; i++) {
		if (harts[i] != mhartid) {
			while (clint_hart_msip_ctls[harts[i]]) {}
			ksem_post(hart_command_que_locks + harts[i]);
		}
	}
	*/
	
	// Return
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
