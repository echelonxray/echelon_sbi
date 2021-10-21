#include "base.h"

struct sbiret sbi_get_spec_version() {
	// Get SBI specification version
	struct sbiret retval;
	retval.value = (   0 << 24) | (   2 <<  0);
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_id() {
	// Get SBI implementation ID
	struct sbiret retval;
	retval.value = 6;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_impl_version() {
	// Get SBI implementation version
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_probe_extension(long extension_id) {
	// Probe SBI extension
	struct sbiret retval;
	if        (extension_id == SBI_EXT_HSM) {
		retval.value = extension_id;
	} else if (0) {
	} else {
		retval.value = 0;
	}
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mvendorid() {
	// Get machine vendor ID
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, mvendorid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_marchid() {
	// Get machine architecture ID
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, marchid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}

struct sbiret sbi_get_mimpid() {
	// Get machine implementation ID
	struct sbiret retval;
	__asm__ __volatile__ ("csrr %0, mimpid" : "=r" (retval.value));
	retval.error = SBI_SUCCESS;
	return retval;
}
