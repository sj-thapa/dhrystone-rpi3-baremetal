//--------------------------------------------------------------------------
// Boot initialization for AArch64 (Raspberry Pi 3B+)
//
// The early entry point (before the stack is available) uses inline
// assembly. Once the stack is set up, control transfers to boot_init()
// which handles the rest in C.
//--------------------------------------------------------------------------

#include <stdint.h>
#include "board.h"

#define CPU_STACK_SIZE 4096

// External symbols
extern uint64_t ttb0_base[];
extern uint64_t mair_value;
extern void init_uart(void) __attribute__((weak));
extern void _program_start(void) __attribute__((weak));
extern void test_main(void) __attribute__((weak));

//--------------------------------------------------------------------------
// Post-stack initialization (called from asm entry after SP is valid)
//--------------------------------------------------------------------------
static void __attribute__((used, section("boot")))
boot_init(void)
{
    // Reset UART hardware before init (warm boot cleanup)
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;
    uart[UART_CR / 4] = 0;

    for (volatile int i = 10000; i > 0; i--);
    __asm__ volatile ("dsb sy\n isb sy");

    if (init_uart) init_uart();

    // Enable NEON if floating-point is present
    uint64_t pfr0;
    __asm__ volatile ("mrs %0, ID_AA64PFR0_EL1" : "=r"(pfr0));

    if (((pfr0 >> 16) & 0xF) == 0) {
        __asm__ volatile (
            "mov    x1, #(0x3 << 20)       \n"
            "msr    cpacr_el1, x1           \n"
            "isb    sy                      \n"
            "fmov   d0,  xzr               \n"
            "fmov   d1,  xzr               \n"
            "fmov   d2,  xzr               \n"
            "fmov   d3,  xzr               \n"
            "fmov   d4,  xzr               \n"
            "fmov   d5,  xzr               \n"
            "fmov   d6,  xzr               \n"
            "fmov   d7,  xzr               \n"
            "fmov   d8,  xzr               \n"
            "fmov   d9,  xzr               \n"
            "fmov   d10, xzr               \n"
            "fmov   d11, xzr               \n"
            "fmov   d12, xzr               \n"
            "fmov   d13, xzr               \n"
            "fmov   d14, xzr               \n"
            "fmov   d15, xzr               \n"
            "fmov   d16, xzr               \n"
            "fmov   d17, xzr               \n"
            "fmov   d18, xzr               \n"
            "fmov   d19, xzr               \n"
            "fmov   d20, xzr               \n"
            "fmov   d21, xzr               \n"
            "fmov   d22, xzr               \n"
            "fmov   d23, xzr               \n"
            "fmov   d24, xzr               \n"
            "fmov   d25, xzr               \n"
            "fmov   d26, xzr               \n"
            "fmov   d27, xzr               \n"
            "fmov   d28, xzr               \n"
            "fmov   d29, xzr               \n"
            "fmov   d30, xzr               \n"
            "fmov   d31, xzr               \n"
            ::: "x1",
                "d0",  "d1",  "d2",  "d3",  "d4",  "d5",  "d6",  "d7",
                "d8",  "d9",  "d10", "d11", "d12", "d13", "d14", "d15",
                "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
                "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"
        );
    }

    // Page table setup
    __asm__ volatile ("msr ttbr0_el3, %0" : : "r"(ttb0_base));

    uint64_t tcr = 0x80803520;
    __asm__ volatile ("msr TCR_EL3, %0" : : "r"(tcr));

    __asm__ volatile ("msr MAIR_EL3, %0" : : "r"(mair_value));

    // Set SMPEN before enabling caches and MMU
    uint64_t cpuectlr;
    __asm__ volatile ("mrs %0, S3_1_C15_C2_1" : "=r"(cpuectlr));
    cpuectlr |= 0x40;
    __asm__ volatile ("msr S3_1_C15_C2_1, %0" : : "r"(cpuectlr));

    // Enable caches and MMU
    uint64_t sctlr;
    __asm__ volatile ("mrs %0, sctlr_el3" : "=r"(sctlr));
    sctlr |= (1 << 2) | (1 << 12) | 1;   // C, I, M bits
    __asm__ volatile ("msr sctlr_el3, %0" : : "r"(sctlr));

    __asm__ volatile ("dsb sy\n isb");

    // Enable interrupts
    __asm__ volatile ("msr DAIFClr, #0xF");

    // Route FIQ/IRQ to EL3
    uint64_t scr;
    __asm__ volatile ("mrs %0, scr_el3" : "=r"(scr));
    scr |= (1 << 2) | (1 << 1);           // FIQ, IRQ bits
    __asm__ volatile ("msr scr_el3, %0" : : "r"(scr));

    __asm__ volatile ("dsb sy\n isb sy");

    // Only CPU0 runs the application; others sleep
    uint64_t mpidr;
    __asm__ volatile ("mrs %0, mpidr_el1" : "=r"(mpidr));

    if ((mpidr & 0xFF) == 0) {
        if (_program_start) _program_start();
        if (test_main) test_main();
    }

    for (;;)
        __asm__ volatile ("wfi");
}

//--------------------------------------------------------------------------
// Entry point - no stack, pure assembly
//
// Invalidates caches/TLBs, zeroes all registers, sets up vector base
// and per-CPU stack pointer, then branches to boot_init().
//--------------------------------------------------------------------------
__asm__(
    ".section boot, \"ax\", %progbits\n"
    ".global init\n\n"

    "init:\n"
    // Invalidate caches and TLBs (warm boot)
    "   ic      iallu               \n"
    "   dsb     sy                  \n"
    "   isb     sy                  \n"
    "   tlbi    alle3               \n"
    "   dsb     sy                  \n"
    "   isb     sy                  \n"

    // Zero general-purpose registers
    "   mov     x0,  xzr            \n"
    "   mov     x1,  xzr            \n"
    "   mov     x2,  xzr            \n"
    "   mov     x3,  xzr            \n"
    "   mov     x4,  xzr            \n"
    "   mov     x5,  xzr            \n"
    "   mov     x6,  xzr            \n"
    "   mov     x7,  xzr            \n"
    "   mov     x8,  xzr            \n"
    "   mov     x9,  xzr            \n"
    "   mov     x10, xzr            \n"
    "   mov     x11, xzr            \n"
    "   mov     x12, xzr            \n"
    "   mov     x13, xzr            \n"
    "   mov     x14, xzr            \n"
    "   mov     x15, xzr            \n"
    "   mov     x16, xzr            \n"
    "   mov     x17, xzr            \n"
    "   mov     x18, xzr            \n"
    "   mov     x19, xzr            \n"
    "   mov     x20, xzr            \n"
    "   mov     x21, xzr            \n"
    "   mov     x22, xzr            \n"
    "   mov     x23, xzr            \n"
    "   mov     x24, xzr            \n"
    "   mov     x25, xzr            \n"
    "   mov     x26, xzr            \n"
    "   mov     x27, xzr            \n"
    "   mov     x28, xzr            \n"
    "   mov     x29, xzr            \n"
    "   mov     x30, xzr            \n"

    // Zero stack pointers, link registers and status registers
    "   mov     sp, x0              \n"
    "   msr     sp_el0, x0          \n"
    "   msr     sp_el1, x0          \n"
    "   msr     sp_el2, x0          \n"
    "   msr     elr_el1, x0         \n"
    "   msr     elr_el2, x0         \n"
    "   msr     elr_el3, x0         \n"
    "   msr     spsr_el1, x0        \n"
    "   msr     spsr_el2, x0        \n"
    "   msr     spsr_el3, x0        \n"

    // Set vector base address
    "   adr     x1, vector_table    \n"
    "   msr     vbar_el3, x1        \n"

    // Set up per-CPU stack pointer
    "   adr     x1, stack_top       \n"
    "   add     x1, x1, #4          \n"
    "   mrs     x2, mpidr_el1       \n"
    "   and     x2, x2, #0xFF       \n"
    "   mov     x3, #4096           \n"
    "   mul     x3, x2, x3          \n"
    "   sub     x1, x1, x3          \n"
    "   mov     sp, x1              \n"

    // Stack is now valid - branch to C
    "   b       boot_init           \n"
);
