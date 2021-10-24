#include "sbi_commands.h"

struct sbiret call_to_sbi(sintRL_t EID, sintRL_t FID, sintRL_t* params) {
	if        (EID == SBI_EXT_BASE) {
		// Base Extension
		DEBUG_print("\tBase Extension\n");
		
		if        (FID == SBI_BASE_GET_SBI_VERSION) {
			// Get SBI specification version
			DEBUG_print("\tGet SBI specification version\n");
			return sbi_get_spec_version();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_ID) {
			// Get SBI implementation ID
			DEBUG_print("\tGet SBI implementation ID\n");
			return sbi_get_impl_id();
		} else if (FID == SBI_BASE_GET_SBI_IMPL_VERSION) {
			// Get SBI implementation version
			DEBUG_print("\tGet SBI implementation version\n");
			return sbi_get_impl_version();
		} else if (FID == SBI_BASE_PROBE_EXTENSION) {
			// Probe SBI extension
			DEBUG_print("\tProbe SBI extension: 0x");
			char str[20];
			itoa(params[0], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			return sbi_probe_extension(params[0]);
		} else if (FID == SBI_BASE_GET_MVENDORID) {
			// Get machine vendor ID
			DEBUG_print("\tGet machine vendor ID\n");
			return sbi_get_mvendorid();
		} else if (FID == SBI_BASE_GET_MARCHID) {
			// Get machine architecture ID
			DEBUG_print("\tGet machine architecture ID\n");
			return sbi_get_marchid();
		} else if (FID == SBI_BASE_GET_MIMPID) {
			// Get machine implementation ID
			DEBUG_print("\tGet machine implementation ID\n");
			return sbi_get_mimpid();
		}
	} else if (EID == SBI_EXT_TIME) {
		// Time Extension		
		DEBUG_print("\tTime Extension\n");
		
		if        (FID == SBI_TIME_SET_TIMER) {
			DEBUG_print("\tSet Timer: ");
			char str[20];
			itoa(params[0], str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print("\n");
			return sbi_set_timer(params[0]);
		}
	} else if (EID == SBI_EXT_IPI) {
		/*
		if        (FID == SBI_IPI_SEND) {
			return sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base);
		}
		*/
	} else if (EID == SBI_EXT_RFNC) {
		/*
		if        (FID == SBI_RFNC_FENCEI) {
			return sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base);
		} else if (FID == SBI_RFNC_SFENCE_VMA) {
			return sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size);
		} else if (FID == SBI_RFNC_SFENCE_VMA_ASID) {
			return sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid);
		} else if (FID == SBI_RFNC_HFENCE_GVMA_VMID) {
			return sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long vmid);
		} else if (FID == SBI_RFNC_HFENCE_GVMA) {
			return sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size);
		} else if (FID == SBI_RFNC_HFENCE_VVMA) {
			return sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid);
		} else if (FID == SBI_RFNC_HFENCE_VVMA_ASID) {
			return sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size);
		}
		*/
	} else if (EID == SBI_EXT_HSM) {
		// Hart State Management Extension
		DEBUG_print("\tHSM Extension\n");
		
		if        (FID == SBI_HSM_HART_START) {
			// Start Hart
			DEBUG_print("\tStart Hart\n");
			
			/*
			char str[20];
			itoa(params[0], str, 20, -10, 0);
			DEBUG_print(str);
			DEBUG_print(" 0x");
			itoa(params[1], str, 20, -16, 0);
			DEBUG_print(str);
			DEBUG_print(" ");
			itoa(params[2], str, 20, -10, 0);
			DEBUG_print(str);
			*/
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
			DEBUG_print("\tStop Hart\n");
			return sbi_hart_stop();
		} else if (FID == SBI_HSM_GET_HART_STATUS) {
			// Get Hart Power Status
			DEBUG_print("\tGet Hart Power Status\n");
			return sbi_hart_get_status(params[0]);
		} else if (FID == SBI_HSM_HART_SUSPEND) {
			// Suspend Hart
			DEBUG_print("\tSuspend Hart\n");
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
