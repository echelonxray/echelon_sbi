// Memory Map of my Custom Javascript Emulator

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

// CLINT
#define CLINT_BASE 0x2000000
//#define CLINT_BASE 0x32000000
//#define CLINT_MTIMECMP 0x00u
//#define CLINT_MTIMECMPHI 0x04u
#define CLINT_MTIMECMPS (0x4000u)
#define CLINT_MTIME (0xBFF8u)
