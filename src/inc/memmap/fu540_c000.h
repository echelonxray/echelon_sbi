// Memory Map of the SiFive FU540-C000 SOC

// Mode Select
#define MODE_SELECT_BASE ((void*)0x00001000u)

// Mask ROM
#define MASK_ROM_BASE ((void*)0x00010000u)

// S51 Management Core - Hart 0: DTIM
#define S51_H0_DTIM_BASE ((void*)0x01000000u)

// S51 Management Core - Hart 0: ITIM
#define S51_H0_ITIM_BASE ((void*)0x01800000u)

// S54 General Purpose Core - Hart 1: ITIM
#define S54_H1_ITIM_BASE ((void*)0x01808000u)

// S54 General Purpose Core - Hart 2: ITIM
#define S54_H2_ITIM_BASE ((void*)0x01810000u)

// S54 General Purpose Core - Hart 3: ITIM
#define S54_H3_ITIM_BASE ((void*)0x01818000u)

// S54 General Purpose Core - Hart 4: ITIM
#define S54_H4_ITIM_BASE ((void*)0x01820000u)

// CLINT
#define CLINT_BASE 0x02000000
//#define CLINT_BASE ((void*)0x02000000u)
// Offsets
#define CLINT_MSIPS (0x0000u)
#define CLINT_H0_MSIP (0x0000u)
#define CLINT_H1_MSIP (0x0004u)
#define CLINT_H2_MSIP (0x0008u)
#define CLINT_H3_MSIP (0x000Cu)
#define CLINT_H4_MSIP (0x0010u)
#define CLINT_MTIMECMPS (0x4000u)
#define CLINT_MTIME (0xBFF8u)

// Cache Controller
#define CACHE_CONTROLLER_BASE ((void*)0x02010000u)

// MSI
#define MSI_BASE ((void*)0x02020000u)

// DMA Controller
#define DMA_CONTROLLER_BASE ((void*)0x03000000u)

// PRCI
#define PRCI_BASE ((void*)0x10000000u)
// Offsets
#define PRCI_HFROSCCFG 0x00u
#define PRCI_HFXOSCCFG 0x04u
#define PRCI_PLLCFG 0x08u
#define PRCI_PLLOUTDIV 0x0Cu
#define PRCI_PROCMONCFG 0xF0u

// SPI Interfaces
#define QSPI0_BASE (0x10040000ul);
#define QSPI1_BASE (0x10041000ul);
#define QSPI2_BASE (0x10050000ul);

// GPIO
#define GPIO_BASE ((void*)0x10060000u)
// Offsets
#define GPIO_INPUT_VAL 0x00u
#define GPIO_INPUT_EN 0x04u
#define GPIO_OUTPUT_EN 0x08u
#define GPIO_OUTPUT_VAL 0x0Cu
#define GPIO_PUE 0x10u
#define GPIO_DS 0x14u
#define GPIO_RISE_IE 0x18u
#define GPIO_RISE_IP 0x1Cu
#define GPIO_FALL_IE 0x20u
#define GPIO_FALL_IP 0x24u
#define GPIO_HIGH_IE 0x28u
#define GPIO_HIGH_IP 0x2Cu
#define GPIO_LOW_IE 0x30u
#define GPIO_LOW_IP 0x34u
#define GPIO_IOF_EN 0x38u
#define GPIO_IOF_SEL 0x3Cu
#define GPIO_OUT_XOR 0x40u

// UART
#define UART0_BASE ((void*)0x10010000u)
#define UART1_BASE ((void*)0x10011000u)
// Offsets
#define UART_TXDATA 0x00u
#define UART_RXDATA 0x04u
#define UART_TXCTRL 0x08u
#define UART_RXCTRL 0x0Cu
#define UART_IE 0x10u
#define UART_IP 0x14u
#define UART_DIV 0x18u
