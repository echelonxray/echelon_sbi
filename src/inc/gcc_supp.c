unsigned int __umodsi3 (unsigned int a, unsigned int b) {
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

unsigned long __umoddi3 (unsigned long a, unsigned long b) {
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

unsigned int __udivsi3 (unsigned int a, unsigned int b) {
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

unsigned long __udivdi3 (unsigned long a, unsigned long b) {
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
