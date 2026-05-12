// UART driver for Raspberry Pi 3B+ (BCM2837)

#include "board.h"

// Mailbox message buffer for setting UART clock
// Must be 16-byte aligned for the mailbox protocol
static volatile unsigned int mbox_buffer[11] __attribute__((aligned(16))) = {
    36,        // 0: Buffer size in bytes
    0,         // 1: Request code
    0x38002,   // 2: Set clock rate tag
    12,        // 3: Value buffer size
    8,         // 4: Request/Response size
    2,         // 5: UART clock ID
    4000000,   // 6: Rate (4MHz)
    0,         // 7: Clear turbo
    0,         // 8: End tag
    0, 0       // 9,10: Padding to 16 bytes
};

// Global UART base address for external reference
volatile unsigned long uart_global_base = UART_BASE;

void init_uart(void)
{
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;
    volatile unsigned int *gpio = (volatile unsigned int *)GPIO_BASE;
    volatile unsigned int *mbox = (volatile unsigned int *)MBOX_BASE;

    // Wait for TX FIFO to empty and UART not busy (important for warm boots)
    while (uart[UART_FR / 4] & 0x08);  // BUSY bit
    while (uart[UART_FR / 4] & 0x20);  // TXFF bit

    // Disable UART before re-initializing
    uart[UART_CR / 4] = 0;
    __asm__ volatile ("dsb sy\nisb sy");

    // Clear any pending interrupts
    uart[UART_ICR / 4] = 0x7FF;

    // Configure GPIO 14,15 to ALT0
    unsigned int sel = gpio[GPFSEL1 / 4];
    sel &= ~(0x7000 | 0x38000);     // Clear bits for GPIO 14, 15
    sel |= (0x4000 | 0x20000);      // Set ALT0 for GPIO 14, 15
    gpio[GPFSEL1 / 4] = sel;

    // Set pull-up/down
    gpio[GPPUD / 4] = 0;
    for (volatile int i = 0; i < 150; i++);

    gpio[GPPUDCLK0 / 4] = 0x6000;
    for (volatile int i = 0; i < 150; i++);

    gpio[GPPUDCLK0 / 4] = 0;

    // Set up UART clock via mailbox
    // Wait for mailbox not full
    while (mbox[MBOX_STATUS / 4] & 0x80000000);

    // Write buffer address (aligned to 16, with channel 8)
    unsigned int addr = ((unsigned int)(unsigned long)&mbox_buffer & ~0xF) | 8;
    mbox[MBOX_WRITE / 4] = addr;

    // Wait for response
    while (mbox[MBOX_STATUS / 4] & 0x40000000);

    // Read and verify response
    while (mbox[MBOX_READ / 4] != addr);

    // Initialize UART registers
    uart[UART_CR / 4] = 0;
    uart[UART_ICR / 4] = 0x7FF;
    uart[UART_IBRD / 4] = 2;
    uart[UART_FBRD / 4] = 0xB;
    uart[UART_LCRH / 4] = 0x70;
    uart[UART_CR / 4] = 0x301;
}

// C implementation of print_string (standard AAPCS: arg in x0)
void print_string_impl(const char *str)
{
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;

    while (*str) {
        char c = *str++;

        // Handle newline: convert to CR+LF
        if (c == '\n') {
            while (uart[UART_FR / 4] & UART_FR_TXFF);
            uart[UART_DR / 4] = '\r';
        }

        // Wait until UART TX FIFO is not full
        while (uart[UART_FR / 4] & UART_FR_TXFF);

        // Send character
        uart[UART_DR / 4] = c;
    }
}

// Assembly shim: vectors.s calls print_string with string pointer in x1
// (non-standard AAPCS). This wrapper moves x1 to x0 and calls the C impl.
__asm__(".global print_string\n"
"print_string:\n"
"    mov x0, x1\n"
"    b print_string_impl\n");

// Internal C implementation of print_char_direct (standard AAPCS: arg in w0)
void print_char_direct_impl(char c)
{
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;

    while (uart[UART_FR / 4] & UART_FR_TXFF);
    uart[UART_DR / 4] = c;
}

// Assembly shim: some callers may pass char in w2 (non-standard)
__asm__(".global print_char_direct\n"
"print_char_direct:\n"
"    mov w0, w2\n"
"    b print_char_direct_impl\n");

void print_newline(void)
{
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;

    while (uart[UART_FR / 4] & UART_FR_TXFF);
    uart[UART_DR / 4] = '\r';

    while (uart[UART_FR / 4] & UART_FR_TXFF);
    uart[UART_DR / 4] = '\n';
}
