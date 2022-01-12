#include "hsm.h"
#include "./../kernel.h"
#include "./../thread_locking.h"
#include "./../interrupts/context_switch.h"

ksemaphore_t* sbi_hsm_locks;
volatile sint32_t* sbi_hsm_states;

extern __thread uintRL_t mhartid;
extern volatile CPU_Context* hart_contexts;

struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque) {
	struct sbiret retval;
	retval.value = 0;
	
	/*
	char buf[20];
	itoa(hartid, buf, 30, -10, 0);
	DEBUG_print("\thartid: ");
	DEBUG_print(buf);
	DEBUG_print("\n");
	*/
	
	if (is_valid_hartid(hartid) == 0) {
		DEBUG_print("\tNot Started: Invalid HartID\n");
		retval.error = SBI_ERR_INVALID_PARAM;
		return retval;
	}
	if (is_valid_phys_mem_addr(start_addr, 2) == 0) {
		DEBUG_print("\tNot Started: Invalid Memory Address\n");
		retval.error = SBI_ERR_INVALID_ADDRESS;
		return retval;
	}
	if (hartid == mhartid) {
		DEBUG_print("\tNot Started: Already Available1\n");
		retval.error = SBI_ERR_ALREADY_AVAILABLE;
		return retval;
	}
	ksem_wait(sbi_hsm_locks + hartid);
	sint32_t hart_state = sbi_hsm_states[hartid];
	if (hart_state == SBI_HSM_STARTED || hart_state == SBI_HSM_START_PENDING || hart_state == SBI_HSM_RESUME_PENDING) {
		DEBUG_print("\tNot Started: Already Available2\n");
		ksem_post(sbi_hsm_locks + hartid);
		retval.error = SBI_ERR_ALREADY_AVAILABLE;
		return retval;
	}
	sbi_hsm_states[hartid] = SBI_HSM_START_PENDING;
	ksem_post(sbi_hsm_locks + hartid);
	Hart_Command command;
	command.command = HARTCMD_STARTHART;
	command.param0 = hartid;
	command.param1 = start_addr;
	command.param2 = opaque;
	send_hart_command_que(hartid, &command);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_hart_stop() {
	DEBUG_print("SBI: HSM_STOP\n");
	ksem_wait(sbi_hsm_locks + mhartid);
	sbi_hsm_states[mhartid] = SBI_HSM_STOPPED;
	ksem_post(sbi_hsm_locks + mhartid);
	switch_context(hart_contexts + mhartid);
	struct sbiret retval;
	retval.value = 0;
	retval.value = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_hart_get_status(unsigned long hartid) {
	DEBUG_print("SBI: HSM_STATUS\n");
	struct sbiret retval;
	if (is_valid_hartid(hartid) == 0) {
		retval.value = 0;
		retval.error = SBI_ERR_INVALID_PARAM;
		return retval;
	}
	retval.value = sbi_hsm_states[hartid];
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_hart_suspend(uint32_t suspend_type, unsigned long resume_addr, unsigned long opaque) {
	DEBUG_print("SBI: HSM_SUSPEND\n");
	struct sbiret retval;
	retval.value = 0;
	if (is_valid_phys_mem_addr(resume_addr, 2) == 0) {
		retval.error = SBI_ERR_INVALID_ADDRESS;
		return retval;
	}
	if (suspend_type == SBI_HSM_SUSPEND_DEFAULT_RETENTIVE || suspend_type == SBI_HSM_SUSPEND_DEFAULT_NONRETENTIVE) {
		hart_contexts[mhartid].regs[REG_A0] = suspend_type;
		hart_contexts[mhartid].regs[REG_A1] = opaque;
		hart_contexts[mhartid].regs[REG_A2] = resume_addr;
		ksem_wait(sbi_hsm_locks + mhartid);
		sbi_hsm_states[mhartid] = SBI_HSM_SUSPENDED;
		ksem_post(sbi_hsm_locks + mhartid);
		switch_context(hart_contexts + mhartid);
	}
	if        (suspend_type >= 0x00000001 && suspend_type <= 0x0FFFFFFF) {
		retval.error = SBI_ERR_INVALID_PARAM;
	} else if (suspend_type >= 0x10000000 && suspend_type <= 0x7FFFFFFF) {
		retval.error = SBI_ERR_NOT_SUPPORTED;
	} else if (suspend_type >= 0x80000001 && suspend_type <= 0x8FFFFFFF) {
		retval.error = SBI_ERR_INVALID_PARAM;
	} else if (suspend_type >= 0x90000000 && suspend_type <= 0xFFFFFFFF) {
		retval.error = SBI_ERR_NOT_SUPPORTED;
	} else {
		retval.error = SBI_ERR_INVALID_PARAM;
	}
	return retval;
}
