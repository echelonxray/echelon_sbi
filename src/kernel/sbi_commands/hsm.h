#ifndef _insertion_kernel_sbic_hsm_h
#define _insertion_kernel_sbic_hsm_h

#include "./../sbi_commands.h"

#define SBI_HSM_HART_START 0
#define SBI_HSM_HART_STOP 1
#define SBI_HSM_GET_HART_STATUS 2
#define SBI_HSM_HART_SUSPEND 3

#define SBI_HSM_STARTED 0
#define SBI_HSM_STOPPED 1
#define SBI_HSM_START_PENDING 2
#define SBI_HSM_STOP_PENDING 3
#define SBI_HSM_SUSPENDED 4
#define SBI_HSM_SUSPEND_PENDING 5
#define SBI_HSM_RESUME_PENDING 6

#define SBI_HSM_SUSPEND_DEFAULT_RETENTIVE 0x00000000
#define SBI_HSM_SUSPEND_DEFAULT_NONRETENTIVE 0x80000000

struct sbiret sbi_hart_start(unsigned long hartid, unsigned long start_addr, unsigned long opaque);
struct sbiret sbi_hart_stop();
struct sbiret sbi_hart_get_status(unsigned long hartid);
struct sbiret sbi_hart_suspend(uint32_t suspend_type, unsigned long resume_addr, unsigned long opaque);

#endif
