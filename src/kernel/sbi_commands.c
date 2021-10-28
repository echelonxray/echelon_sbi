#include "sbi_commands.h"
#include "./kernel.h"

extern uintRL_t load_point;

extern __thread uintRL_t mhartid;

struct sbiret call_to_sbi(sintRL_t EID, sintRL_t FID, sintRL_t* params) {
	if        (EID == SBI_EXT_BASE) {
		// Base Extension
		//DEBUG_print("\tBase Extension\n");

		if        (FID == SBI_BASE_GET_SBI_VERSION) {
			// Get SBI specification version
			//DEBUG_print("\tGet SBI specification version\n");
			return sbi_get_spec_version();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_ID) {
			// Get SBI implementation ID
			//DEBUG_print("\tGet SBI implementation ID\n");
			return sbi_get_impl_id();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_VERSION) {
			// Get SBI implementation version
			//DEBUG_print("\tGet SBI implementation version\n");
			return sbi_get_impl_version();
		} else if (FID == SBI_BASE_PROBE_EXTENSION) {
			// Probe SBI extension
			/*
			DEBUG_print("\tProbe SBI extension: 0x");
			char str[20];
			itoa(params[0], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			*/
			return sbi_probe_extension(params[0]);
		} else if (FID == SBI_BASE_GET_MVENDORID) {
			// Get machine vendor ID
			//DEBUG_print("\tGet machine vendor ID\n");
			return sbi_get_mvendorid();
		} else if (FID == SBI_BASE_GET_MARCHID) {
			// Get machine architecture ID
			//DEBUG_print("\tGet machine architecture ID\n");
			return sbi_get_marchid();
		} else if (FID == SBI_BASE_GET_MIMPID) {
			// Get machine implementation ID
			//DEBUG_print("\tGet machine implementation ID\n");
			return sbi_get_mimpid();
		}
	} else if (EID == SBI_EXT_TIME) {
		// Time Extension
		//DEBUG_print("\tTime Extension\n");

		if        (FID == SBI_TIME_SET_TIMER) {
			/*
			DEBUG_print("\tSet Timer: ");
			char str[20];
			itoa(params[0], str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			*/
			return sbi_set_timer(params[0]);
		}
	} else if (EID == SBI_EXT_IPI) {
		// IPI Extension
		//DEBUG_print("\tIPI Extension\n");
		
		if        (FID == SBI_IPI_SEND_IPI) {
			/*
			char str[20];
			DEBUG_print("Send IPI [Hart: ");
			itoa(mhartid, str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print("] 0x");
			itoa(params[0], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print(" 0x");
			itoa(params[1], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			*/
			return sbi_send_ipi(params[0], params[1]);
		}
	} else if (EID == SBI_EXT_RFNC) {
		// Remote Fence Extension
		//DEBUG_print("\tRFNC Extension\n");

		if        (FID == SBI_RFNC_FENCEI) {
			//DEBUG_print("\tFence.I\n");
			return sbi_remote_fence_i(params[0], params[1]);
		} else if (FID == SBI_RFNC_SFENCE_VMA) {
			//DEBUG_print("\tSFence.VMA\n");
			return sbi_remote_sfence_vma(params[0], params[1], params[2], params[3]);
		} else if (FID == SBI_RFNC_SFENCE_VMA_ASID) {
			//DEBUG_print("\tSFence.VMA_ASID\n");
			return sbi_remote_sfence_vma_asid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_GVMA_VMID) {
			//DEBUG_print("\tHFence.GVMA_VMID\n");
			return sbi_remote_hfence_gvma_vmid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_GVMA) {
			//DEBUG_print("\tHFence.GVMA\n");
			return sbi_remote_hfence_gvma(params[0], params[1], params[2], params[3]);
		} else if (FID == SBI_RFNC_HFENCE_VVMA_ASID) {
			//DEBUG_print("\tHFence.VVMA_ASID\n");
			return sbi_remote_hfence_vvma_asid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_VVMA) {
			//DEBUG_print("\tHFence.VVMA\n");
			return sbi_remote_hfence_vvma(params[0], params[1], params[2], params[3]);
		}
	} else if (EID == SBI_EXT_HSM) {
		// Hart State Management Extension
		//DEBUG_print("\tHSM Extension\n");

		if        (FID == SBI_HSM_HART_START) {
			// Start Hart

			char str[20];
			DEBUG_print("Start Hart: ");
			itoa(params[0], str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print(" 0x");
			itoa(params[1], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print(" ");
			itoa(params[2], str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			struct sbiret sbir = sbi_hart_start(params[0], params[1], params[2]);
			/*
			DEBUG_print(" E ");
			itoa(sbir.error, str, 20, 10, 0);
			DEBUG_print(str);
			DEBUG_print(" R ");
			itoa(sbir.value, str, 20, 10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			*/
			return sbir;
		} else if (FID == SBI_HSM_HART_STOP) {
			// Stop Hart
			return sbi_hart_stop();
		} else if (FID == SBI_HSM_GET_HART_STATUS) {
			// Get Hart Power Status
			//DEBUG_print("\tGet Hart Power Status\n");
			return sbi_hart_get_status(params[0]);
		} else if (FID == SBI_HSM_HART_SUSPEND) {
			// Suspend Hart
			//DEBUG_print("\tSuspend Hart\n");
			return sbi_hart_suspend(params[0], params[1], params[2]);
		}
	} else if (EID == SBI_EXT_SRST) {
		/*
		if        (FID == SBI_SRST_RESET) {
			return sbi_system_reset(uint32_t reset_type, uint32_t reset_reason);
		}
		*/
	}

	// Not Supported
	char str[30];
	DEBUG_print("ESBI Error.  Not Supported: ");
	itoa(EID, str, 30, -16, 0);
	DEBUG_print(str);
	DEBUG_print(" x ");
	itoa(FID, str, 30, -16, 0);
	DEBUG_print(str);
	DEBUG_print("\n");
	struct sbiret retval;
	retval.value = 0;
	retval.error = SBI_ERR_NOT_SUPPORTED;
	return retval;
}

uintRL_t is_valid_phys_mem_addr(uintRL_t address, uintRL_t required_alignment) {
	if (address & ((1ul << required_alignment) - 1)) {
		// Misaligned
		return 0;
	}
	if (address < MEMORY_BASE || address >= (MEMORY_BASE + MEMORY_AVAILABLE)) {
		// Out of range
		return 0;
	}
	// Is valid
	return 1;
}

uintRL_t is_valid_hartid(uintRL_t hartid) {
	if (hartid < (TOTAL_HART_COUNT - USE_HART_COUNT) || hartid >= TOTAL_HART_COUNT) {
		// Out of range
		return 0;
	}
	// Is valid
	return 1;
}
