#ifndef _insertion_kernel_sbic_h
#define _insertion_kernel_sbic_h

#include "./../inc/types.h"
#include "./../inc/string.h"
#include "./inc/general_oper.h"
#include "./sbi_commands/base.h"
#include "./sbi_commands/hsm.h"
#include "./debug.h"

#define SBI_SUCCESS 0
#define SBI_ERR_FAILED -1
#define SBI_ERR_NOT_SUPPORTED -2
#define SBI_ERR_INVALID_PARAM -3
#define SBI_ERR_DENIED -4
#define SBI_ERR_INVALID_ADDRESS -5
#define SBI_ERR_ALREADY_AVAILABLE -6
#define SBI_ERR_ALREADY_STARTED -7
#define SBI_ERR_ALREADY_STOPPED -8

#define SBI_EXT_BASE 0x10
#define SBI_EXT_TIME 0x54494D45
#define SBI_EXT_IPI 0x735049
#define SBI_EXT_RFNC 0x52464E43
#define SBI_EXT_HSM 0x48534D
#define SBI_EXT_SRST 0x53525354
#define SBI_EXT_PMU 0x504D55

struct sbiret {
	long error;
	long value;
};

struct sbiret call_to_sbi(sintRL_t EID, sintRL_t FID, sintRL_t* params);

#endif
