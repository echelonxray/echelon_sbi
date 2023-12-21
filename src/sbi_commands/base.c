#include "base.h"
#include <inc/csr.h>

struct sbiret sbi_get_spec_version() {
	// Get SBI specification version
	
	printm("\tGet SBI specification version\n");
	
	struct sbiret retval;
	retval.value = (   0 << 24) | (   3 <<  0);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_id() {
	// Get SBI implementation ID
	
	printm("\tGet SBI implementation ID\n");
	
	struct sbiret retval;
	retval.value = 6;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_version() {
	// Get SBI implementation version
	
	printm("\tGet SBI implementation version\n");
	
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_probe_extension(long extension_id) {
	// Probe SBI extension
	
	printm("\tProbe SBI extension\n");
	
	struct sbiret retval;
	if        (extension_id == SBI_EXT_BASE) {
		
		printm("\t\tSBI_EXT_BASE\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_TIME) {
		
		printm("\t\tSBI_EXT_TIME\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_IPI) {
		
		printm("\t\tSBI_EXT_IPI\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_RFNC) {
		
		printm("\t\tSBI_EXT_RFNC\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_HSM) {
		
		printm("\t\tSBI_EXT_HSM\n");
		
		// Returns 1 if exists in QEMU OpenSBI implementation
		retval.value = 1;
	} else if (extension_id == SBI_EXT_SRST) {
		
		printm("\t\tSBI_EXT_SRST\n");
		
		// Not implementation in QEMU OpenSBI
		retval.value = 0;
	} else if (extension_id == SBI_EXT_PMU) {
		
		printm("\t\tSBI_EXT_PMU\n");
		
		// Not implementation in QEMU OpenSBI
		retval.value = 0;
	} else {
		
		printm("\t\tNot Implemented\n");
		
		// Return 0 if not implemented
		retval.value = 0;
	}
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mvendorid() {
	// Get machine vendor ID
	
	printm("\tGet machine vendor ID\n");
	
	struct sbiret retval;
	retval.value = CSRI_BITCLR(CSR_MVENDORID, 0);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_marchid() {
	// Get machine architecture ID
	
	printm("\tGet machine architecture ID\n");
	
	struct sbiret retval;
	retval.value = CSRI_BITCLR(CSR_MARCHID, 0);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mimpid() {
	// Get machine implementation ID
	
	printm("\tGet machine implementation ID\n");
	
	struct sbiret retval;
	retval.value = CSRI_BITCLR(CSR_MIMPID, 0);
	retval.error = SBI_SUCCESS;
	return retval;
}
