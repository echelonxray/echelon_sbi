#ifndef _insertion_sbi_commands_h
#define _insertion_sbi_commands_h

#include <string.h>
#include <debug.h>
#include <inc/types.h>
#include <inc/reg.h>
#include "sbi_commands/base.h"
#include "sbi_commands/time.h"
#include "sbi_commands/ipi.h"
#include "sbi_commands/rfnc.h"
#include "sbi_commands/hsm.h"

#define SBI_SUCCESS                0
#define SBI_ERR_FAILED            -1
#define SBI_ERR_NOT_SUPPORTED     -2
#define SBI_ERR_INVALID_PARAM     -3
#define SBI_ERR_DENIED            -4
#define SBI_ERR_INVALID_ADDRESS   -5
#define SBI_ERR_ALREADY_AVAILABLE -6
#define SBI_ERR_ALREADY_STARTED   -7
#define SBI_ERR_ALREADY_STOPPED   -8

#define SBI_EXT_BASE              0x00000010
#define SBI_EXT_TIME              0x54494D45
#define SBI_EXT_IPI               0x00735049
#define SBI_EXT_RFNC              0x52464E43
#define SBI_EXT_HSM               0x0048534D
#define SBI_EXT_SRST              0x53525354
#define SBI_EXT_PMU               0x00504D55

struct sbiret {
	long error;
	long value;
};

struct sbiret call_to_sbi(sintRL_t EID, sintRL_t FID, sintRL_t* params);
uintRL_t is_valid_phys_mem_addr(uintRL_t address, uintRL_t required_alignment);
uintRL_t is_valid_hartid(uintRL_t hartid);

#endif
