unsigned int __umodsi3 (unsigned int a, unsigned int b) {
	while (a >= b) {
		a -= b;
	}
	return a;
}

unsigned long __umoddi3 (unsigned long a, unsigned long b) {
	while (a >= b) {
		a -= b;
	}
	return a;
}

unsigned int __udivsi3 (unsigned int a, unsigned int b) {
	unsigned int i = 0;
	while (a >= b) {
		a -= b;
		i++;
	}
	return i;
}

unsigned long __udivdi3 (unsigned long a, unsigned long b) {
	unsigned long i = 0;
	while (a >= b) {
		a -= b;
		i++;
	}
	return i;
}
