#include "base.h"

struct sbiret sbi_get_spec_version() {
	// Get SBI specification version
	
	DEBUG_print("\tGet SBI specification version\n");
	
	struct sbiret retval;
	retval.value = (   0 << 24) | (   3 <<  0);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_id() {
	// Get SBI implementation ID
	
	DEBUG_print("\tGet SBI implementation ID\n");
	
	struct sbiret retval;
	retval.value = 6;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_version() {
	// Get SBI implementation version
	
	DEBUG_print("\tGet SBI implementation version\n");
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_probe_extension(long extension_id) {
	// Probe SBI extension
	
	DEBUG_print("\tProbe SBI extension\n");
	
	struct sbiret retval;
	if        (extension_id == SBI_EXT_BASE) {
		
		DEBUG_print("\t\tSBI_EXT_BASE\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_TIME) {
		
		DEBUG_print("\t\tSBI_EXT_TIME\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_IPI) {
		
		DEBUG_print("\t\tSBI_EXT_IPI\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_RFNC) {
		
		DEBUG_print("\t\tSBI_EXT_RFNC\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_HSM) {
		
		DEBUG_print("\t\tSBI_EXT_HSM\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_SRST) {
		
		DEBUG_print("\t\tSBI_EXT_SRST\n");
		
		// Not implementation in QEMU OpenSBI
		retval.value = 0;
	} else if (extension_id == SBI_EXT_PMU) {
		
		DEBUG_print("\t\tSBI_EXT_PMU\n");
		
		// Not implementation in QEMU OpenSBI
		retval.value = 0;
	} else {
		
		DEBUG_print("\t\tNot Implemented\n");
		
		// Return 0 if not implemented
		retval.value = 0;
	}
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mvendorid() {
	// Get machine vendor ID
	
	DEBUG_print("\tGet machine vendor ID\n");
	
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, mvendorid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_marchid() {
	// Get machine architecture ID
	
	DEBUG_print("\tGet machine architecture ID\n");
	
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, marchid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mimpid() {
	// Get machine implementation ID
	
	DEBUG_print("\tGet machine implementation ID\n");
	
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, mimpid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}
