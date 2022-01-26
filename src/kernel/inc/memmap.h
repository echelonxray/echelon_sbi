#ifndef _insertion_gpio_map_h
#define _insertion_gpio_map_h

#ifdef MM_FE310_GOO2
  #include "./memmaps/fe310_g002.h"
#elif  MM_FU540_C000
  #include "./memmaps/fu540_c000.h"
#elif  MM_QEMU_VIRT
  #include "./memmaps/fu540_c000.h"
#elif  MM_CUSTOM_EMU
  #include "./memmaps/cust_emu_000.h"
#else
  #error No device memory map specificed in the preprocessor definitions
#endif

#endif
