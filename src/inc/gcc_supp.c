#include "types.h"

/*
So my problem was not, in fact, a compiler bug.  It was incorrect 
documentation.  The GNU manual page here: 
https://gcc.gnu.org/onlinedocs/gccint/Integer-library-routines.html 
describes these functions in terms of signed and unsigned; int, long, 
and long long.  However, the RISC-V port of the GCC complier does 
not follow that standard.  I'm not sure if the RISC-V port is unique 
in the regard or if that page is simply wrong, possibly being 
architecture specific without saying it.  

Per the RISC-V type sizes, listed in the RISC-V Calling Convention 
specifications:
- int:       32-bits wide.
- long:      XLEN-bits wide.  It matches the register width.
- long long: 64-bits wide.

However the RISC-V GCC compiler considers functions to be like this: 
- __***si*(): 32-bits
- __***di*(): 64-bits
- __***ti*(): 128-bits (As far as I know, this is not implemented 
                        on RISC-V.  At least not on rv32)

Thank you for the help: 
https://github.com/riscv-collab/riscv-gcc/issues/324
*/

sint32_t __mulsi3(sint32_t a, sint32_t b) {
	uint32_t ua = a;
	uint32_t ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	uint32_t j = 0;
	for (uint32_t i = 0; i < sizeof(sint32_t) * 8; i++) {
		if (!ua || !ub) {
			break;
		}
		if (ua & 0x1) {
			j += ub;
		}
		ua >>= 1;
		ub <<= 1;
	}
	
	sint32_t r = j;
	a ^= b;
	if (a < 0) {
		return -r;
	}
	return r;
}

sint32_t __modsi3(sint32_t a, sint32_t b) {
	uint32_t ua = a;
	uint32_t ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	uint32_t i = 0;
	while (ua >= ub) {
		if ((sint32_t)ub < 0) {
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
	
	sint32_t r = ua;
	if (a < 0) {
		return -r;
	}
	return r;
}

uint32_t __umodsi3(uint32_t a, uint32_t b) {
	uint32_t i = 0;
	while (a >= b) {
		if ((sint32_t)b < 0) {
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

uint64_t __umoddi3(uint64_t a, uint64_t b) {
	uint64_t i = 0;
	while (a >= b) {
		if ((signed long)b < 0) {
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

sint32_t __divsi3(sint32_t a, sint32_t b) {
	uint32_t ua = a;
	uint32_t ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	uint32_t i = 0;
	uint32_t j = 0;
	while (ua >= ub) {
		if ((sint32_t)ub < 0) {
			ua -= ub;
			j |= 1 << i;
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
			j |= 1 << i;
		}
	}
	
	sint32_t r = j;
	a ^= b;
	if (a < 0) {
		return -r;
	}
	return r;
}

uint32_t __udivsi3(uint32_t a, uint32_t b) {
	uint32_t i = 0;
	uint32_t j = 0;
	while (a >= b) {
		if ((sint32_t)b < 0) {
			a -= b;
			j |= 1 << i;
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
			j |= 1 << i;
		}
	}
	return j;
}

uint64_t __udivdi3(uint64_t a, uint64_t b) {
	uint64_t i = 0;
	uint64_t j = 0;
	while (a >= b) {
		if ((signed long)b < 0) {
			a -= b;
			j |= 1ul << i;
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
			j |= 1ul << i;
		}
	}
	return j;
}

/*
unsigned long long __umodti3(unsigned long long a, unsigned long long b) {
	unsigned long i = 0;
	while (a >= b) {
		if ((signed long long)b < 0) {
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

unsigned long long __udivti3(unsigned long long a, unsigned long long b) {
	unsigned long i = 0;
	unsigned long long j = 0;
	while (a >= b) {
		if ((signed long long)b < 0) {
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
*/
