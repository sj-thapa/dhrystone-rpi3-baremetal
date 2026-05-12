// Board definitions for AArch64 assembly sources
// Raspberry Pi 3B+ (BCM2837)

// Peripheral MMIO base
.equ MMIO_BASE,     0x3F000000

// PL011 UART
.equ UART_BASE,     0x3F201000
.equ UART_DR,       0x00
.equ UART_FR,       0x18
.equ UART_IBRD,     0x24
.equ UART_FBRD,     0x28
.equ UART_LCRH,     0x2C
.equ UART_CR,       0x30
.equ UART_IMSC,     0x38
.equ UART_ICR,      0x44
.equ UART_TXFF,     1 << 5

// GPIO
.equ GPIO_BASE,     0x3F200000
.equ GPFSEL1,       0x04
.equ GPPUD,         0x94
.equ GPPUDCLK0,     0x98

// Mailbox
.equ MBOX_BASE,     0x3F00B880
.equ MBOX_READ,     0x00
.equ MBOX_STATUS,   0x18
.equ MBOX_WRITE,    0x20
.equ MBOX_EMPTY,    0x40000000
.equ MBOX_FULL,     0x80000000
