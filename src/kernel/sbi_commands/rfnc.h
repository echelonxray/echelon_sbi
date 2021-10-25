#ifndef _insertion_kernel_sbic_rfnc_h
#define _insertion_kernel_sbic_rfnc_h

#include "./../sbi_commands.h"

#define SBI_RFNC_FENCEI 0
#define SBI_RFNC_SFENCE_VMA 1
#define SBI_RFNC_SFENCE_VMA_ASID 2
#define SBI_RFNC_HFENCE_GVMA_VMID 3
#define SBI_RFNC_HFENCE_GVMA 4
#define SBI_RFNC_HFENCE_VVMA_ASID 5
#define SBI_RFNC_HFENCE_VVMA 6

struct sbiret sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base);
struct sbiret sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size);
struct sbiret sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid);
struct sbiret sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long vmid);
struct sbiret sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size);
struct sbiret sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid);
struct sbiret sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size);

#endif
