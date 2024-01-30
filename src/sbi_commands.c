#include "sbi_commands.h"
#include <main.h>

extern uintRL_t load_point;

extern __thread uintRL_t mhartid;

struct sbiret call_to_sbi(sintRL_t EID, sintRL_t FID, sintRL_t* params) {
	if        (EID == SBI_EXT_BASE) {
		// Base Extension

		if        (FID == SBI_BASE_GET_SBI_VERSION) {
			// Get SBI specification version
			return sbi_get_spec_version();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_ID) {
			// Get SBI implementation ID
			return sbi_get_impl_id();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_VERSION) {
			// Get SBI implementation version
			return sbi_get_impl_version();
		} else if (FID == SBI_BASE_PROBE_EXTENSION) {
			// Probe SBI extension
			return sbi_probe_extension(params[0]);
		} else if (FID == SBI_BASE_GET_MVENDORID) {
			// Get machine vendor ID
			return sbi_get_mvendorid();
		} else if (FID == SBI_BASE_GET_MARCHID) {
			// Get machine architecture ID
			return sbi_get_marchid();
		} else if (FID == SBI_BASE_GET_MIMPID) {
			// Get machine implementation ID
			return sbi_get_mimpid();
		}
	} else if (EID == SBI_EXT_TIME) {
		// Time Extension

		if        (FID == SBI_TIME_SET_TIMER) {
			#if   __riscv_xlen == 128
				#error "SBI Time Extension: Not supported on 128-bit XLEN"
			#elif __riscv_xlen == 64
				return sbi_set_timer(params[0]);
			#else
				uint64_t tmp_param;
				tmp_param = (uint32_t)params[1];
				tmp_param <<= 32;
				tmp_param |= (uint32_t)params[0];
				return sbi_set_timer(tmp_param);
			#endif
		}
	} else if (EID == SBI_EXT_IPI) {
		// IPI Extension
		
		if        (FID == SBI_IPI_SEND_IPI) {
			return sbi_send_ipi(params[0], params[1]);
		}
	} else if (EID == SBI_EXT_RFNC) {
		// Remote Fence Extension

		if        (FID == SBI_RFNC_FENCEI) {
			return sbi_remote_fence_i(params[0], params[1]);
		} else if (FID == SBI_RFNC_SFENCE_VMA) {
			return sbi_remote_sfence_vma(params[0], params[1], params[2], params[3]);
		} else if (FID == SBI_RFNC_SFENCE_VMA_ASID) {
			return sbi_remote_sfence_vma_asid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_GVMA_VMID) {
			return sbi_remote_hfence_gvma_vmid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_GVMA) {
			return sbi_remote_hfence_gvma(params[0], params[1], params[2], params[3]);
		} else if (FID == SBI_RFNC_HFENCE_VVMA_ASID) {
			return sbi_remote_hfence_vvma_asid(params[0], params[1], params[2], params[3], params[4]);
		} else if (FID == SBI_RFNC_HFENCE_VVMA) {
			return sbi_remote_hfence_vvma(params[0], params[1], params[2], params[3]);
		}
	} else if (EID == SBI_EXT_HSM) {
		// Hart State Management Extension

		if        (FID == SBI_HSM_HART_START) {
			// Start Hart
			return sbi_hart_start(params[0], params[1], params[2]);
		} else if (FID == SBI_HSM_HART_STOP) {
			// Stop Hart
			return sbi_hart_stop();
		} else if (FID == SBI_HSM_GET_HART_STATUS) {
			// Get Hart Power Status
			return sbi_hart_get_status(params[0]);
		} else if (FID == SBI_HSM_HART_SUSPEND) {
			// Suspend Hart
			return sbi_hart_suspend(params[0], params[1], params[2]);
		}
	} else if (EID == SBI_EXT_SRST) {
		// System Reset

		if        (FID == SBI_SRST_RESET) {
			// System Reset
			return sbi_system_reset(params[0], params[1]);
		}
	}

	// Unsupported SBI Request.
	printm("ESBI Error.  Unsupported SBI Request.  EID: 0x%08lX, FID: 0x%08lX\n", (uintRL_t)EID, (uintRL_t)FID);
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
