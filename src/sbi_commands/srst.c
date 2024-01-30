#include "srst.h"
#include <main.h>
#include <thread_locking.h>
#include <interrupts/context_switch.h>

extern __thread uintRL_t mhartid;
extern volatile CPU_Context* hart_contexts;

struct sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason) {
	struct sbiret retval;
	retval.value = 0;

	if (reset_reason != 0) {
		retval.error = SBI_ERR_INVALID_PARAM;
		return retval;
	}

	if        (reset_type == SBI_SRST_SHUTDOWN) {
		printm("ESBI: SRST: Shutdown\n");
#ifdef MM_CUSTOM_EMU
		volatile uint32_t* power_ctl = (void*)0x100000; // Echelon EMU Power Control Device
		uint32_t command = 0x2; // Shutdown
		*power_ctl = command;
#endif
#ifdef MM_QEMU_VIRT
		volatile uint32_t* test_dev = (void*)0x100000; // QEMU SiFive Test Device
		uint32_t code = 0; // Normal
		uint32_t command = 0x5555; // Shutdown
		*test_dev = (code << 16) | (command <<  0);
#endif
	} else if (reset_type == SBI_SRST_COLD_REBOOT) {
		printm("ESBI: SRST: Cold Reboot\n");
#ifdef MM_CUSTOM_EMU
		volatile uint32_t* power_ctl = (void*)0x100000; // Echelon EMU Power Control Device
		uint32_t command = 0x1; // Reset
		*power_ctl = command;
#endif
#ifdef MM_QEMU_VIRT
		volatile uint32_t* test_dev = (void*)0x100000; // QEMU SiFive Test Device
		uint32_t code = 0; // Normal
		uint32_t command = 0x7777; // Reset
		*test_dev = (code << 16) | (command <<  0);
#endif
	} else if (reset_type == SBI_SRST_WARM_REBOOT) {
		printm("ESBI: SRST: Warm Reboot\n");
#ifdef MM_CUSTOM_EMU
		volatile uint32_t* power_ctl = (void*)0x100000; // Echelon EMU Power Control Device
		uint32_t command = 0x1; // Reset
		*power_ctl = command;
#endif
#ifdef MM_QEMU_VIRT
		volatile uint32_t* test_dev = (void*)0x100000; // QEMU SiFive Test Device
		uint32_t code = 0; // Normal
		uint32_t command = 0x7777; // Reset
		*test_dev = (code << 16) | (command <<  0);
#endif
	} else {
		retval.error = SBI_ERR_INVALID_PARAM;
		return retval;
	}

	retval.error = SBI_ERR_FAILED;
	return retval;
}
