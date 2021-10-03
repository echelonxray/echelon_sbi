#ifndef _insertion_gpio_map_h
#define _insertion_gpio_map_h

// GPIO
#define GPIO_BASE ((void*)0x10012000u)
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

// AON
#define AON_BASE ((void*)0x10000000u)
// Offsets
#define AON_WDOGCFG 0x000u
#define AON_WDOGCOUNT 0x008u
#define AON_WDOGS 0x010u
#define AON_WDOGFEED 0x018u
#define AON_WDOGKEY 0x01Cu
#define AON_WDOGCMP0 0x020u
#define AON_RTCCFG 0x040u
#define AON_RTCCOUNTLO 0x048u
#define AON_RTCCOUNTHI 0x04Cu
#define AON_RTCS 0x050u
#define AON_RTCCMP0 0x060u
#define AON_LFROSCCFG 0x070u
#define AON_LFCLKMUX 0x07Cu
#define AON_BACKUP_0 0x080u
#define AON_BACKUP_1 0x084u
#define AON_BACKUP_2 0x088u
#define AON_BACKUP_3 0x08Cu
#define AON_BACKUP_4 0x090u
#define AON_BACKUP_5 0x094u
#define AON_BACKUP_6 0x098u
#define AON_BACKUP_7 0x09Cu
#define AON_BACKUP_8 0x0A0u
#define AON_BACKUP_9 0x0A4u
#define AON_BACKUP_10 0x0A8u
#define AON_BACKUP_11 0x0ACu
#define AON_BACKUP_12 0x0B0u
#define AON_BACKUP_13 0x0B4u
#define AON_BACKUP_14 0x0B8u
#define AON_BACKUP_15 0x0B0u
#define AON_PMUWAKEUPI0 0x100u
#define AON_PMUWAKEUPI1 0x104u
#define AON_PMUWAKEUPI2 0x108u
#define AON_PMUWAKEUPI3 0x10Cu
#define AON_PMUWAKEUPI4 0x110u
#define AON_PMUWAKEUPI5 0x114u
#define AON_PMUWAKEUPI6 0x118u
#define AON_PMUWAKEUPI7 0x11Cu
#define AON_PMUSLEEPI0 0x120u
#define AON_PMUSLEEPI1 0x124u
#define AON_PMUSLEEPI2 0x128u
#define AON_PMUSLEEPI3 0x12Cu
#define AON_PMUSLEEPI4 0x130u
#define AON_PMUSLEEPI5 0x134u
#define AON_PMUSLEEPI6 0x138u
#define AON_PMUSLEEPI7 0x13Cu
#define AON_PMUIE 0x140u
#define AON_PMUCAUSE 0x144u
#define AON_PMUSLEEP 0x148u
#define AON_PMUKEY 0x14Cu
#define AON_SIFIVEBANDGAP 0x210u
#define AON_AONCFG 0x300u

// CLINT
#define CLINT_BASE ((void*)0x02000000u)
// Offsets
#define CLINT_MSIP 0x0000u
#define CLINT_MTIMECMP 0x4000u
#define CLINT_MTIMECMP_LO 0x4000u
#define CLINT_MTIMECMP_HI 0x4004u
#define CLINT_MTIME 0xBFF8u
#define CLINT_MTIME_LO 0xBFF8u
#define CLINT_MTIME_HI 0xBFFCu

// PRCI
#define PRCI_BASE ((void*)0x10008000u)
// Offsets
#define PRCI_HFROSCCFG 0x00u
#define PRCI_HFXOSCCFG 0x04u
#define PRCI_PLLCFG 0x08u
#define PRCI_PLLOUTDIV 0x0Cu
#define PRCI_PROCMONCFG 0xF0u

#endif
