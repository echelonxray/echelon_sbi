#include "hsm.h"
#include "./../thread_locking.h"

ksemaphore_t* sbi_hsm_locks;

struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque) {
	struct sbiret retval;
	return retval;
}

struct sbiret sbi_hart_stop() {
	struct sbiret retval;
	return retval;
}

struct sbiret sbi_hart_get_status(unsigned long hartid) {
	struct sbiret retval;
	return retval;
}

struct sbiret sbi_hart_suspend(uint32_t suspend_type, unsigned long resume_addr, unsigned long opaque) {
	struct sbiret retval;
	return retval;
}
