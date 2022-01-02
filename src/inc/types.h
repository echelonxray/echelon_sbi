#ifndef _insertion_types_h
#define _insertion_types_h

typedef unsigned long long int uint64_t;
typedef signed long long int sint64_t;
typedef signed long long int int64_t;

typedef unsigned int uint32_t;
typedef signed int sint32_t;
typedef signed int int32_t;

typedef unsigned short int uint16_t;
typedef signed short int sint16_t;
typedef signed short int int16_t;

typedef unsigned char uint8_t;
typedef signed char sint8_t;
typedef signed char int8_t;

#if   __riscv_xlen == 128
typedef uint128_t uintRL_t;
typedef sint128_t sintRL_t;
typedef sint128_t intRL_t;
#elif __riscv_xlen == 64
typedef uint64_t uintRL_t;
typedef sint64_t sintRL_t;
typedef sint64_t intRL_t;
#else
typedef uint32_t uintRL_t;
typedef sint32_t sintRL_t;
typedef sint32_t intRL_t;
#endif

typedef uintRL_t size_t;
typedef uintRL_t usize_t;
typedef sintRL_t ssize_t;

#endif

