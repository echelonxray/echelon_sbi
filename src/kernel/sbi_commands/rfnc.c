#include "rfnc.h"
#include "./../kernel.h"
#include "./../interrupts/context_switch.h"
#include "./../inc/memmap.h"

extern __thread uintRL_t mhartid;

struct sbiret sbi_remote_fence_i(unsigned long hart_mask, unsigned long hart_mask_base) {
  uintRL_t harts[32];
  uintRL_t hart_count = 0;

  // Identify hearts to command
  if (hart_mask_base == (unsigned long)-1) {
    for (uintRL_t i = TOTAL_HART_COUNT - USE_HART_COUNT; i < TOTAL_HART_COUNT; i++) {
      harts[hart_count] = i;
      hart_count++;
    }
    goto iterate_the_harts;
  }
  for (uintRL_t i = 0; i < 32; i++) {
    if (hart_mask & 0x1) {
      uintRL_t hartid = hart_mask_base + hart_count;
      if (is_valid_hartid(hartid) == 0) {
        struct sbiret retval;
      	retval.value = 0;
      	retval.error = SBI_ERR_INVALID_PARAM;
        return retval;
      }
      harts[hart_count] = hartid;
      hart_count++;
    }
    hart_mask >>= 1;
  }

  iterate_the_harts:
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] == mhartid) {
      __asm__ __volatile__ ("fence.i");
    } else {
      Hart_Command command;
      command.command = HARTCMD_REMOTE_FENCE_I;
      command.param0 = harts[i];
      send_hart_command_que(harts[i], &command);
    }
  }
  volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] != mhartid) {
      while (clint_hart_msip_ctls[harts[i]]) {}
    }
  }

  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_SUCCESS;
  return retval;
}
struct sbiret sbi_remote_sfence_vma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, unsigned long size) {
  uintRL_t harts[32];
  uintRL_t hart_count = 0;

  // Identify hearts to command
  if (hart_mask_base == (unsigned long)-1) {
    for (uintRL_t i = TOTAL_HART_COUNT - USE_HART_COUNT; i < TOTAL_HART_COUNT; i++) {
      harts[hart_count] = i;
      hart_count++;
    }
    goto iterate_the_harts;
  }
  for (uintRL_t i = 0; i < 32; i++) {
    if (hart_mask & 0x1) {
      uintRL_t hartid = hart_mask_base + hart_count;
      if (is_valid_hartid(hartid) == 0) {
        struct sbiret retval;
      	retval.value = 0;
      	retval.error = SBI_ERR_INVALID_PARAM;
        return retval;
      }
      harts[hart_count] = hartid;
      hart_count++;
    }
    hart_mask >>= 1;
  }

  iterate_the_harts:
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] == mhartid) {
      // TODO: Specific parameters
      __asm__ __volatile__ ("sfence.vma zero, zero");
    } else {
      Hart_Command command;
      command.command = HARTCMD_REMOTE_SFENCE_VMA;
      command.param0 = harts[i];
      command.param1 = start_addr;
      command.param2 = size;
      send_hart_command_que(harts[i], &command);
    }
  }
  volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] != mhartid) {
      while (clint_hart_msip_ctls[harts[i]]) {}
    }
  }

  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_SUCCESS;
  return retval;
}
struct sbiret sbi_remote_sfence_vma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid) {
  uintRL_t harts[32];
  uintRL_t hart_count = 0;

  // Identify hearts to command
  if (hart_mask_base == (unsigned long)-1) {
    for (uintRL_t i = TOTAL_HART_COUNT - USE_HART_COUNT; i < TOTAL_HART_COUNT; i++) {
      harts[hart_count] = i;
      hart_count++;
    }
    goto iterate_the_harts;
  }
  for (uintRL_t i = 0; i < 32; i++) {
    if (hart_mask & 0x1) {
      uintRL_t hartid = hart_mask_base + hart_count;
      if (is_valid_hartid(hartid) == 0) {
        struct sbiret retval;
      	retval.value = 0;
      	retval.error = SBI_ERR_INVALID_PARAM;
        return retval;
      }
      harts[hart_count] = hartid;
      hart_count++;
    }
    hart_mask >>= 1;
  }

  iterate_the_harts:
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] == mhartid) {
      // TODO: Specific parameters
      __asm__ __volatile__ ("sfence.vma zero, zero");
    } else {
      Hart_Command command;
      command.command = HARTCMD_REMOTE_SFENCE_VMA_ASID;
      command.param0 = harts[i];
      command.param1 = start_addr;
      command.param2 = size;
      command.param3 = asid;
      send_hart_command_que(harts[i], &command);
    }
  }
  volatile uint32_t* clint_hart_msip_ctls = (uint32_t*)CLINT_BASE;
  for (uintRL_t i = 0; i < hart_count; i++) {
    if (harts[i] != mhartid) {
      while (clint_hart_msip_ctls[harts[i]]) {}
    }
  }

  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_SUCCESS;
  return retval;
}
struct sbiret sbi_remote_hfence_gvma_vmid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long vmid) {
  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_ERR_NOT_SUPPORTED;
  return retval;
}
struct sbiret sbi_remote_hfence_gvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size) {
  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_ERR_NOT_SUPPORTED;
  return retval;
}
struct sbiret sbi_remote_hfence_vvma_asid(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size, unsigned long asid) {
  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_ERR_NOT_SUPPORTED;
  return retval;
}
struct sbiret sbi_remote_hfence_vvma(unsigned long hart_mask, unsigned long hart_mask_base, unsigned long start_addr, long size) {
  struct sbiret retval;
  retval.value = 0;
  retval.error = SBI_ERR_NOT_SUPPORTED;
  return retval;
}
