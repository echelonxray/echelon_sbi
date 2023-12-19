#ifndef _insertion_sbi_commands_ipi_h
#define _insertion_sbi_commands_ipi_h

#include <sbi_commands.h>

#define SBI_IPI_SEND_IPI 0

struct sbiret sbi_send_ipi(unsigned long hart_mask, unsigned long hart_mask_base);

#endif
