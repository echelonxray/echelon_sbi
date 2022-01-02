#include "ipi.h"
#include "./../kernel.h"
#include "./../interrupts/context_switch.h"
#include "./../inc/memmap.h"
#include "./../thread_locking.h"

extern ksemaphore_t* hart_command_que_locks;
extern __thread uintRL_t mhartid;

struct sbiret sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base) {
	uintRL_t harts[32];
	uintRL_t hart_count = 0;
	
	// Identify hearts to command
	if (hart_mask_base == (unsigned long)-1) {
		for (uintRL_t i = TOTAL_HART_COUNT - USE_HART_COUNT; i < TOTAL_HART_COUNT; i++) {
			harts[hart_count] = i;
			hart_count++;
		}
		goto iterate_the_harts;
	}
	
	for (uintRL_t i = 0; i < 32; i++) {
		if (hart_mask & 0x1) {
			uintRL_t hartid = hart_mask_base + hart_count;
			if (is_valid_hartid(hartid) == 0) {
				DEBUG_print("SBI FAILURE: SEND_IPI\n");
				struct sbiret retval;
				retval.value = 0;
				retval.error = SBI_ERR_INVALID_PARAM;
				return retval;
			}
			harts[hart_count] = hartid;
			hart_count++;
		}
		hart_mask >>= 1;
	}
	
	iterate_the_harts:
	for (uintRL_t i = 0; i < hart_count; i++) {
		if (harts[i] != mhartid) {
			Hart_Command command;
			command.command = HARTCMD_SMODE_SOFTINT;
			command.param0 = harts[i];
			send_hart_command_que(harts[i], &command);
			//send_hart_command_lck(harts[i], &command);
		} else {
			__asm__ __volatile__ ("csrs mip, %0" : : "r" (0x2));
		}
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
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}
