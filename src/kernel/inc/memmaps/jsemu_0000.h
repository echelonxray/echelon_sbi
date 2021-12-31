// Memory Map of the SiFive FE310-G002 SOC

// UART
#define UART0_BASE ((void*)0x10000000u)
// Offsets
#define UART_TXDATA 0x00u
#define UART_RXDATA 0x04u
#define UART_TXCTRL 0x08u
#define UART_RXCTRL 0x0Cu
#define UART_IE 0x10u
#define UART_IP 0x14u
#define UART_DIV 0x18u
