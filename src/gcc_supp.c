/*
 * Copyright (C) 2022 Michael T. Kloos <michael@michaelkloos.com>
 */

/*
 * The GNU manual page here:
 * https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html
 * describes these functions in terms of signed and unsigned, int and long.
 * However, these prototypes are wrong when the size of int and long are
 * considered per the RISC-V ABI specification.  The RISC-V port of the GCC
 * complier does not follow the standard of those prototypes.  This is
 * discussed in this thread:
 * https://github.com/riscv-collab/riscv-gcc/issues/324
 *
 * On the RISC-V Architecture:
 * - __xyyysi3 always refers to  32-bit integers.
 * - __xyyydi3 always refers to  64-bit integers.
 * - __xyyyti3 always refers to 128-bit integers. (Only implemented for rv64)
 *
 * Per the RISC-V ABI specification, the C base types are:
 * - int:       32-bits wide.
 * - long:      XLEN-bits wide.  It matches the register width.
 * - long long: 64-bits wide.
 *
 * Therefore, the correct RISC-V function prototypes are:
 * - signed int __mulsi3(signed int a, signed int b);
 * - signed long long __muldi3(signed long long a, signed long long b);
 * - signed __int128 __multi3(signed __int128 a, signed __int128 b);
 *
 * - signed int __divsi3(signed int a, signed int b);
 * - signed long long __divdi3(signed long long a, signed long long b);
 * - signed __int128 __divti3(signed __int128 a, signed __int128 b);
 *
 * - unsigned int __udivsi3(unsigned int a, unsigned int b);
 * - unsigned long long __udivdi3(unsigned long long a, unsigned long long b);
 * - unsigned __int128 __udivti3(unsigned __int128 a, unsigned __int128 b);
 *
 * - signed int __modsi3(signed int a, signed int b);
 * - signed long long __moddi3(signed long long a, signed long long b);
 * - signed __int128 __modti3(signed __int128 a, signed __int128 b);
 *
 * - unsigned int __umodsi3(unsigned int a, unsigned int b);
 * - unsigned long long __umoddi3(unsigned long long a, unsigned long long b);
 * - unsigned __int128 __umodti3(unsigned __int128 a, unsigned __int128 b);
 */

#ifndef __riscv_mul

#if __riscv_xlen > 64
#error "Unsupported XLEN"
#endif

#define UINT_MSB (((unsigned       int)1) <<  31)
#define ULL_MSB  (((unsigned long long)1) <<  63)
#if __riscv_xlen >= 64
#define U128_MSB (((unsigned  __int128)1) << 127)
#endif

signed int __mulsi3(signed int a, signed int b) {
	unsigned int ua;
	unsigned int ub;
	unsigned int j;
	unsigned int i;
	signed int r;

	ua = a;
	ub = b;

	j = 0;
	for (i = 0; i < sizeof(signed int) * 8; i++) {
		if (!ua || !ub) {
			break;
		}
		if (ua & 0x1) {
			j += ub;
		}
		ua >>= 1;
		ub <<= 1;
	}

	r = j;

	return r;
}

signed long long __muldi3(signed long long a, signed long long b) {
	unsigned long long ua;
	unsigned long long ub;
	unsigned long long j;
	unsigned int i;
	signed long long r;

	ua = a;
	ub = b;

	j = 0;
	for (i = 0; i < sizeof(signed long long) * 8; i++) {
		if (!ua || !ub) {
			break;
		}
		if (ua & 0x1) {
			j += ub;
		}
		ua >>= 1;
		ub <<= 1;
	}

	r = j;

	return r;
}

#if __riscv_xlen >= 64
signed __int128 __multi3(signed __int128 a, signed __int128 b) {
	unsigned __int128 ua;
	unsigned __int128 ub;
	unsigned __int128 j;
	unsigned int i;
	signed __int128 r;

	ua = a;
	ub = b;

	j = 0;
	for (i = 0; i < sizeof(signed __int128) * 8; i++) {
		if (!ua || !ub) {
			break;
		}
		if (ua & 0x1) {
			j += ub;
		}
		ua >>= 1;
		ub <<= 1;
	}

	r = j;

	return r;
}
#endif

signed int __divsi3(signed int a, signed int b) {
	unsigned int ua;
	unsigned int ub;
	unsigned int j;
	unsigned int i;
	signed int r;

	if (b == 0) {
		return (signed int)(-1);
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	j = 0;
	i = 0;
	while (ua >= ub) {
		if (ub & UINT_MSB) {
			ua -= ub;
			j |= 1u << i;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
			j |= 1u << i;
		}
	}

	r = j;
	if ((a < 0) != ((b < 0))) {
		r = -r;
	}

	return r;
}

signed long long __divdi3(signed long long a, signed long long b) {
	unsigned long long ua;
	unsigned long long ub;
	unsigned long long j;
	unsigned int i;
	signed long long r;

	if (b == 0) {
		return (signed long long)(-1);
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	j = 0;
	i = 0;
	while (ua >= ub) {
		if (ub & ULL_MSB) {
			ua -= ub;
			j |= 1ull << i;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
			j |= 1ull << i;
		}
	}

	r = j;
	if ((a < 0) != ((b < 0))) {
		r = -r;
	}

	return r;
}

#if __riscv_xlen >= 64
signed __int128 __divti3(signed __int128 a, signed __int128 b) {
	unsigned __int128 ua;
	unsigned __int128 ub;
	unsigned __int128 j;
	unsigned int i;
	signed __int128 r;

	if (b == 0) {
		return (signed __int128)(-1);
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	j = 0;
	i = 0;
	while (ua >= ub) {
		if (ub & U128_MSB) {
			ua -= ub;
			j |= ((unsigned __int128)1) << i;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
			j |= ((unsigned __int128)1) << i;
		}
	}

	r = j;
	if ((a < 0) != ((b < 0))) {
		r = -r;
	}

	return r;
}
#endif

unsigned int __udivsi3(unsigned int a, unsigned int b) {
	unsigned int j;
	unsigned int i;

	if (b == 0) {
		return (signed int)(-1);
	}

	j = 0;
	i = 0;
	while (a >= b) {
		if (b & UINT_MSB) {
			a -= b;
			j |= 1u << i;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
			j |= 1u << i;
		}
	}

	return j;
}

unsigned long long __udivdi3(unsigned long long a, unsigned long long b) {
	unsigned long long j;
	unsigned long long i;

	if (b == 0) {
		return (signed long long)(-1);
	}

	j = 0;
	i = 0;
	while (a >= b) {
		if (b & ULL_MSB) {
			a -= b;
			j |= 1ull << i;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
			j |= 1ull << i;
		}
	}

	return j;
}

#if __riscv_xlen >= 64
unsigned __int128 __udivti3(unsigned __int128 a, unsigned __int128 b) {
	unsigned __int128 j;
	unsigned __int128 i;

	if (b == 0) {
		return (signed __int128)(-1);
	}

	j = 0;
	i = 0;
	while (a >= b) {
		if (b & U128_MSB) {
			a -= b;
			j |= ((unsigned __int128)1) << i;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
			j |= ((unsigned __int128)1) << i;
		}
	}

	return j;
}
#endif

signed int __modsi3(signed int a, signed int b) {
	unsigned int ua;
	unsigned int ub;
	unsigned int i;
	signed int r;

	if (b == 0) {
		return a;
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	i = 0;
	while (ua >= ub) {
		if (ub & UINT_MSB) {
			ua -= ub;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
		}
	}

	r = ua;
	if (a < 0) {
		r = -r;
	}

	return r;
}

signed long long __moddi3(signed long long a, signed long long b) {
	unsigned long long ua;
	unsigned long long ub;
	unsigned int i;
	signed long long r;

	if (b == 0) {
		return a;
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	i = 0;
	while (ua >= ub) {
		if (ub & ULL_MSB) {
			ua -= ub;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
		}
	}

	r = ua;
	if (a < 0) {
		r = -r;
	}

	return r;
}

#if __riscv_xlen >= 64
signed __int128 __modti3(signed __int128 a, signed __int128 b) {
	unsigned __int128 ua;
	unsigned __int128 ub;
	unsigned int i;
	signed __int128 r;

	if (b == 0) {
		return a;
	}

	ua = a;
	ub = b;
	if (a < 0) {
		ua = -a;
	}
	if (b < 0) {
		ub = -b;
	}

	i = 0;
	while (ua >= ub) {
		if (ub & U128_MSB) {
			ua -= ub;
			break;
		}
		ub <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		ub >>= 1;
		if (ua >= ub) {
			ua -= ub;
		}
	}

	r = ua;
	if (a < 0) {
		r = -r;
	}

	return r;
}
#endif

unsigned int __umodsi3(unsigned int a, unsigned int b) {
	unsigned int i;

	if (b == 0) {
		return a;
	}

	i = 0;
	while (a >= b) {
		if (b & UINT_MSB) {
			a -= b;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
		}
	}

	return a;
}

unsigned long long __umoddi3(unsigned long long a, unsigned long long b) {
	unsigned long long i;

	if (b == 0) {
		return a;
	}

	i = 0;
	while (a >= b) {
		if (b & ULL_MSB) {
			a -= b;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
		}
	}

	return a;
}

#if __riscv_xlen >= 64
unsigned __int128 __umodti3(unsigned __int128 a, unsigned __int128 b) {
	unsigned __int128 i;

	if (b == 0) {
		return a;
	}

	i = 0;
	while (a >= b) {
		if (b & U128_MSB) {
			a -= b;
			break;
		}
		b <<= 1;
		i++;
	}
	while (i > 0) {
		i--;
		b >>= 1;
		if (a >= b) {
			a -= b;
		}
	}

	return a;
}
#endif

#endif
