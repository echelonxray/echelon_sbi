#ifndef _insertion_cpu_h
#define _insertion_cpu_h

#define CPU_WAIT() \
	{	__asm__ __volatile__ ("wfi");	}

#endif
