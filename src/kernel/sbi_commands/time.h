#ifndef _insertion_kernel_sbic_time_h
#define _insertion_kernel_sbic_time_h

#include "./../sbi_commands.h"

struct sbiret sbi_set_timer(uint64_t stime_value);

#define SBI_TIME_SET_TIMER 0

#endif
