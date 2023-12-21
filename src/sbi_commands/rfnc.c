#include "rfnc.h"
#include <main.h>
#include <thread_locking.h>
#include <inc/memmap.h>
#include <inc/cpu.h>
#include <interrupts/context_switch.h>

extern ksemaphore_t* hart_command_que_locks;
extern __thread uintRL_t mhartid;

struct sbiret sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base) {
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
				printm("SBI FAILURE: Remote Fence.I\n");
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
				CPU_FENCEI();
			} else {
				Hart_Command command;
				command.command = HARTCMD_REMOTE_FENCE_I;
				command.param0 = hartid;
				send_hart_command_que(hartid, &command);
			}
		}
		tmp_hart_mask >>= 1;
	}
	
	// Return
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
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
				printm("SBI FAILURE: Remote SFence.VMA\n");
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
				// TODO: Specific parameters
				CPU_SFENCEVMA();
			} else {
				Hart_Command command;
				command.command = HARTCMD_REMOTE_SFENCE_VMA;
				command.param0 = hartid;
				command.param1 = start_addr;
				command.param2 = size;
				send_hart_command_que(hartid, &command);
			}
		}
		tmp_hart_mask >>= 1;
	}
	
	// Return
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid) {
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
				printm("SBI FAILURE: Remote SFence.VMA_ASID\n");
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
			// TODO: Specific parameters
			CPU_SFENCEVMA();
			} else {
				Hart_Command command;
				command.command = HARTCMD_REMOTE_SFENCE_VMA_ASID;
				command.param0 = hartid;
				command.param1 = start_addr;
				command.param2 = size;
				command.param3 = asid;
				send_hart_command_que(hartid, &command);
			}
		}
		tmp_hart_mask >>= 1;
	}
	
	// Return
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

struct sbiret sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long vmid) {
	printm("SBI FAILURE: HFence.GVMA_VMID\n");
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_ERR_NOT_SUPPORTED;
	return retval;
}

struct sbiret sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size) {
	printm("SBI FAILURE: HFence.GVMA\n");
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_ERR_NOT_SUPPORTED;
	return retval;
}

struct sbiret sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid) {
	printm("SBI FAILURE: HFence.VVMA_ASID\n");
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_ERR_NOT_SUPPORTED;
	return retval;
}

struct sbiret sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size) {
	printm("SBI FAILURE: HFence.VVMA\n");
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_ERR_NOT_SUPPORTED;
	return retval;
}

#pragma GCC diagnostic pop
