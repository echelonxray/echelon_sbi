#ifndef _insertion_inc_cpu_h
#define _insertion_inc_cpu_h

#define CPU_WAIT() \
	{	__asm__ __volatile__ ("wfi \n");	}

#define CPU_FENCEI() \
	{	__asm__ __volatile__ ("fence.i \n");	}

#define CPU_SFENCEVMA() \
	{	__asm__ __volatile__ ("sfence.vma zero, zero \n");	}

#endif
