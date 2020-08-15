#ifndef _insertion_gpio_oper_h
#define _insertion_gpio_oper_h

#include "types.h"
#include "memmap.h"

// Set GPIO IOF mode for pin
//   func == 0: No special IOF mode. Normal GPIO operation.
//   func == 1: 1st special IOF mode (IOF0).  GPIO operation suppressed.
//   func == 2: 2nd special IOF mode (IOF1).  GPIO operation suppressed.
//   func == [Other]: GPIO_IOF_EN is set for the pin, but GPIO_IOF_SEL is unchanged.
// 
// Not all pins have special IOF mode and some only have the 1st OR 2nd.
// Invalid selections silently revert to Normal GPIO operation (func == 0).
// Valid IOF mode selections are listed in the FE310-G002 v19p05 manual 
// under Section 17.8 in Table 53.
#define IOF_SEL(pin, func) \
	{ \
		if (func == 0) { \
			(*(volatile uint32_t*)(GPIO_BASE + GPIO_IOF_EN)) &= ~(1u << pin); \
		} else { \
			if (func == 1) { \
				(*(volatile uint32_t*)(GPIO_BASE + GPIO_IOF_SEL)) &= ~(1u << pin); \
			} else if (func == 2) { \
				(*(volatile uint32_t*)(GPIO_BASE + GPIO_IOF_SEL)) |= (1u << pin); \
			} \
			(*(volatile uint32_t*)(GPIO_BASE + GPIO_IOF_EN)) |= (1u << pin); \
		} \
	}
#define INPUT(pin) \
	{	(*(volatile uint32_t*)(GPIO_BASE + GPIO_INPUT_EN)) |= (1u << pin);	}
#define OUTPUT(pin) \
	{	(*(volatile uint32_t*)(GPIO_BASE + GPIO_OUTPUT_EN)) |= (1u << pin);	}
#define PIN_VAL(pin, val) \
	{ \
		if ((*(volatile uint32_t*)(GPIO_BASE + GPIO_INPUT_VAL)) & (1u << pin)) { \
			val = 1; \
		} else { \
			val = 0; \
		} \
	}
#define HIGH(pin) \
	{	*(volatile uint32_t*)(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1u << pin);	}
#define LOW(pin) \
	{	*(volatile uint32_t*)(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1u << pin);	}

#endif
