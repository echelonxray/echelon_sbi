signed int __mulsi3(signed int a, signed int b) {
	unsigned int ua = a;
	unsigned int ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	unsigned int j = 0;
	for (unsigned int i = 0; i < sizeof(signed int) * 8; i++) {
		if (!ua || !ub) {
			break;
		}
		if (ua & 0x1) {
			j += ub;
		}
		ua >>= 1;
		ub <<= 1;
	}
	
	signed int r = j;
	a ^= b;
	if (a < 0) {
		return -r;
	}
	return r;
}

signed int __modsi3(signed int a, signed int b) {
	unsigned int ua = a;
	unsigned int ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	unsigned int i = 0;
	while (ua >= ub) {
		if ((signed int)ub < 0) {
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
	
	signed int r = ua;
	if (a < 0) {
		return -r;
	}
	return r;
}

unsigned int __umodsi3(unsigned int a, unsigned int b) {
	unsigned int i = 0;
	while (a >= b) {
		if ((signed int)b < 0) {
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

unsigned long __umoddi3(unsigned long a, unsigned long b) {
	unsigned long i = 0;
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

signed int __divsi3(signed int a, signed int b) {
	unsigned int ua = a;
	unsigned int ub = b;
	if (a < 0) {
		ua = ~ua + 1;
	}
	if (b < 0) {
		ub = ~ub + 1;
	}
	
	unsigned int i = 0;
	unsigned int j = 0;
	while (ua >= ub) {
		if ((signed int)ub < 0) {
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
	
	signed int r = j;
	a ^= b;
	if (a < 0) {
		return -r;
	}
	return r;
}

unsigned int __udivsi3(unsigned int a, unsigned int b) {
	unsigned int i = 0;
	unsigned int j = 0;
	while (a >= b) {
		if ((signed int)b < 0) {
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

unsigned long __udivdi3(unsigned long a, unsigned long b) {
	unsigned long i = 0;
	unsigned long j = 0;
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

unsigned long long __umodti3(unsigned long long a, unsigned long long b) {
	unsigned long i = 0;
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

unsigned long long __udivti3(unsigned long long a, unsigned long long b) {
	unsigned long i = 0;
	unsigned long long j = 0;
	while (a >= b) {
		if ((signed long)b < 0) {
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
