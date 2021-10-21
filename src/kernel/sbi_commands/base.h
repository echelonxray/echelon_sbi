#ifndef _insertion_kernel_sbic_base_h
#define _insertion_kernel_sbic_base_h

#include "./../sbi_commands.h"

#define SBI_BASE_GET_SBI_VERSION 0
#define SBI_BASE_GET_SBI_IMPL_ID 1
#define SBI_BASE_GET_SBI_IMPL_VERSION 2
#define SBI_BASE_PROBE_EXTENSION 3
#define SBI_BASE_GET_MVENDORID 4
#define SBI_BASE_GET_MARCHID 5
#define SBI_BASE_GET_MIMPID 6

struct sbiret sbi_get_spec_version();
struct sbiret sbi_get_impl_id();
struct sbiret sbi_get_impl_version();
struct sbiret sbi_probe_extension(long extension_id);
struct sbiret sbi_get_mvendorid();
struct sbiret sbi_get_marchid();
struct sbiret sbi_get_mimpid();

#endif
