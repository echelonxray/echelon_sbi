#ifndef _insertion_types_h
#define _insertion_types_h

typedef unsigned int uint32_t;
typedef signed int sint32_t;
typedef signed int int32_t;

#if __riscv_xlen == 128
typedef uint128_t uintRL_t;
typedef sint128_t sintRL_t;
#elsif __riscv_flen == 64
typedef uint64_t uintRL_t;
typedef sint64_t sintRL_t;
#else
typedef uint32_t uintRL_t;
typedef sint32_t sintRL_t;
#endif

typedef uintRL_t size_t;
typedef sintRL_t ssize_t;

#endif
