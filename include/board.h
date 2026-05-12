#ifndef BOARD_H
#define BOARD_H

/*
 * Raspberry Pi 3B+ (BCM2837) Board Definitions
 *
 * All peripheral base addresses and register offsets for the
 * BCM2837 SoC used on the Raspberry Pi 3 Model B+.
 */

/* Peripheral MMIO base (mapped via VideoCore GPU) */
#define MMIO_BASE       0x3F000000

/* GPIO */
#define GPIO_BASE       (MMIO_BASE + 0x00200000)
#define GPFSEL1         0x04
#define GPPUD           0x94
#define GPPUDCLK0       0x98

/* PL011 UART */
#define UART_BASE       (MMIO_BASE + 0x00201000)
#define UART_DR         0x00
#define UART_FR         0x18
#define UART_IBRD       0x24
#define UART_FBRD       0x28
#define UART_LCRH       0x2C
#define UART_CR         0x30
#define UART_IMSC       0x38
#define UART_ICR        0x44
#define UART_FR_TXFF    (1 << 5)

/* Mailbox */
#define MBOX_BASE       (MMIO_BASE + 0x0000B880)
#define MBOX_READ       0x00
#define MBOX_STATUS     0x18
#define MBOX_WRITE      0x20

#endif /* BOARD_H */
