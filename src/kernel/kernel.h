#ifndef _insertion_kernel_h
#define _insertion_kernel_h

#ifdef MM_FU540_C000
#define TOTAL_HART_COUNT 2
#define USE_HART_COUNT 1
#endif

#ifdef MM_JSEMU_0000
#define TOTAL_HART_COUNT 1
#define USE_HART_COUNT 1
#endif

#define STACK_SIZE 0x1000
#define MEMORY_BASE 0x80000000ul
#define MEMORY_AVAILABLE 0x40000000ul // 0x4000_0000 (1 GiB)

#endif
