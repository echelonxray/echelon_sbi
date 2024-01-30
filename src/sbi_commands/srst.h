#ifndef _insertion_sbi_commands_srst_h
#define _insertion_sbi_commands_srst_h

#include <sbi_commands.h>

#define SBI_SRST_RESET 0

#define SBI_SRST_SHUTDOWN 0
#define SBI_SRST_COLD_REBOOT 1
#define SBI_SRST_WARM_REBOOT 2

struct sbiret sbi_system_reset(uint32_t reset_type, uint32_t reset_reason);

#endif
