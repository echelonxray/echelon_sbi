#ifndef _insertion_inc_memmap_h
#define _insertion_inc_memmap_h

#ifdef MM_FE310_GOO2
  #include "memmap/fe310_g002.h"
#elif  MM_FU540_C000
  #include "memmap/fu540_c000.h"
#elif  MM_QEMU_VIRT
  #include "memmap/cust_emu_000.h"
#elif  MM_CUSTOM_EMU
  #include "memmap/cust_emu_000.h"
#else
  #error "No device memory map specificed in the preprocessor definitions"
#endif

#endif
