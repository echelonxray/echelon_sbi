#ifndef _insertion_general_oper_h
#define _insertion_general_oper_h

#define  pc  0

#define  x1  1
#define  x2  2
#define  x3  3
#define  x4  4
#define  x5  5
#define  x6  6
#define  x7  7
#define  x8  8
#define  x9  9
#define x10 10
#define x11 11
#define x12 12
#define x13 13
#define x14 14
#define x15 15
#define x16 16
#define x17 17
#define x18 18
#define x19 19
#define x20 20
#define x21 21
#define x22 22
#define x23 23
#define x24 24
#define x25 25
#define x26 26
#define x27 27
#define x28 28
#define x29 29
#define x30 30
#define x31 31

#define  ra  x1
#define  sp  x2
#define  gp  x3
#define  tp  x4
#define  t0  x5
#define  t1  x6
#define  t2  x7

// x8 has 2 different names
#define  s0  x8
#define  fp  x8

#define  s1  x9
#define  a0 x10
#define  a1 x11
#define  a2 x12
#define  a3 x13
#define  a4 x14
#define  a5 x15
#define  a6 x16
#define  a7 x17
#define  s2 x18
#define  s3 x19
#define  s4 x20
#define  s5 x21
#define  s6 x22
#define  s7 x23
#define  s8 x24
#define  s9 x25
#define s10 x26
#define s11 x27
#define  t3 x28
#define  t4 x29
#define  t5 x30
#define  t6 x31

#define CPU_WAIT() \
	{	__asm__ __volatile__ ("wfi");	}

#define CPU_RELAX() \
	{	__asm__ __volatile__ ("nop"); \
		__asm__ __volatile__ ("nop"); \
		__asm__ __volatile__ ("nop"); \
		__asm__ __volatile__ ("nop"); \
		__asm__ __volatile__ ("nop");	}

#define ENABLE_SOFTWARE_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrs zero, mie, %0" : : "r" (0x00000008u));	}

#define DISABLE_SOFTWARE_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrc zero, mie, %0" : : "r" (0x00000008u));	}

#define ENABLE_TIMER_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrs zero, mie, %0" : : "r" (0x00000080u));	}

#define DISABLE_TIMER_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrc zero, mie, %0" : : "r" (0x00000080u));	}

#define ENABLE_EXTERNAL_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrs zero, mie, %0" : : "r" (0x00000800u));	}

#define DISABLE_EXTERNAL_INTERRUPT() \
	{	__asm__ __volatile__ ("csrrc zero, mie, %0" : : "r" (0x00000800u));	}

#endif
