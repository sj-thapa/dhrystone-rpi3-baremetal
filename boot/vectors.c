#include <stdint.h>

//--------------------------------------------------------------------------
// Exception vector table for AArch64
//
// The vector table uses inline assembly for hardware-mandated alignment
// and weak-symbol branch semantics. All handler logic is in C.
//
// When a weak handler symbol is undefined the linker converts the
// branch to NOP, so execution falls through to the C default handler.
//--------------------------------------------------------------------------

// print_string_impl is defined in uart.c with standard AAPCS (arg in x0)
extern void print_string_impl(const char *str);

// Simple hex print for diagnostic registers
static void __attribute__((used, section("vectors")))
print_hex(unsigned long val)
{
    char buf[19]; // "0x" + 16 hex digits + null
    const char hex[] = "0123456789abcdef";
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        buf[2 + (15 - i)] = hex[(val >> (i * 4)) & 0xF];
    }
    buf[18] = '\0';
    print_string_impl(buf);
}

static void __attribute__((noreturn, used, section("vectors")))
terminate(void)
{
    print_string_impl("** EXCEPTION OCCURRED! **\n");
    for (;;);
}

//--------------------------------------------------------------------------
// Default handlers for each exception vector.
// Each prints the exception name and terminates.
//--------------------------------------------------------------------------

#define DEFAULT_HANDLER(name, msg)                          \
    static void __attribute__((noreturn, used, section("vectors"))) \
    default_##name(void)                                    \
    {                                                       \
        uint64_t esr, elr, far_val;                         \
        __asm__ volatile("mrs %0, esr_el3" : "=r"(esr));   \
        __asm__ volatile("mrs %0, elr_el3" : "=r"(elr));   \
        __asm__ volatile("mrs %0, far_el3" : "=r"(far_val)); \
        print_string_impl("Unexpected exception: " msg);   \
        print_string_impl("  ESR_EL3="); print_hex(esr);   \
        print_string_impl("  ELR_EL3="); print_hex(elr);   \
        print_string_impl("  FAR_EL3="); print_hex(far_val); \
        print_string_impl("\n");                            \
        terminate();                                        \
    }

DEFAULT_HANDLER(sp0_sync,       "sp0_sync\n")
DEFAULT_HANDLER(sp0_irq,        "sp0_irq\n")
DEFAULT_HANDLER(sp0_fiq,        "sp0_fiq\n")
DEFAULT_HANDLER(sp0_serror,     "sp0_serror\n")
DEFAULT_HANDLER(spx_sync,       "spx_sync\n")
DEFAULT_HANDLER(spx_irq,        "spx_irq\n")
DEFAULT_HANDLER(spx_fiq,        "spx_fiq\n")
DEFAULT_HANDLER(spx_serror,     "spx_serror\n")
DEFAULT_HANDLER(a64_sync,       "a64_sync\n")
DEFAULT_HANDLER(a64_irq,        "a64_irq\n")
DEFAULT_HANDLER(a64_fiq,        "a64_fiq\n")
DEFAULT_HANDLER(a64_serror,     "a64_serror\n")
DEFAULT_HANDLER(a32_sync,       "a32_sync\n")
DEFAULT_HANDLER(a32_irq,        "a32_irq\n")
DEFAULT_HANDLER(a32_fiq,        "a32_fiq\n")
DEFAULT_HANDLER(a32_serror,     "a32_serror\n")

//--------------------------------------------------------------------------
// Vector table
//
// Each 128-byte slot:
//   b <weak_handler>       - linker turns this into NOP if undefined
//   b <default_handler>    - C fallback (only reached when weak is NOP)
//   .balign 0x80           - pad to next 128-byte boundary
//--------------------------------------------------------------------------

#define VECTOR_ENTRY(label, weak_sym, default_fn) \
    #label ":\n"                                \
    ".weak " #weak_sym "\n"                     \
    "b " #weak_sym "\n"                         \
    "b " #default_fn "\n"                       \
    ".balign 0x80\n"

__asm__(
    ".section vectors, \"ax\", %progbits\n"
    ".balign 0x800\n"
    ".global vector_table\n"
    "vector_table:\n"

    // Current EL with SP0
    VECTOR_ENTRY(vec_sp0_sync,   vec_curr_el_sp0_sync,       default_sp0_sync)
    VECTOR_ENTRY(vec_sp0_irq,    vec_curr_el_sp0_irq,        default_sp0_irq)
    VECTOR_ENTRY(vec_sp0_fiq,    vec_curr_el_sp0_fiq,        default_sp0_fiq)
    VECTOR_ENTRY(vec_sp0_serror, vec_curr_el_sp0_serror,     default_sp0_serror)

    // Current EL with SPx
    VECTOR_ENTRY(vec_spx_sync,   vec_curr_el_spx_sync,       default_spx_sync)
    VECTOR_ENTRY(vec_spx_irq,    vec_curr_el_spx_irq,        default_spx_irq)
    VECTOR_ENTRY(vec_spx_fiq,    vec_curr_el_spx_fiq,        default_spx_fiq)
    VECTOR_ENTRY(vec_spx_serror, vec_curr_el_spx_serror,     default_spx_serror)

    // Lower EL using AArch64
    VECTOR_ENTRY(vec_a64_sync,   vec_lower_el_aarch64_sync,  default_a64_sync)
    VECTOR_ENTRY(vec_a64_irq,    vec_lower_el_aarch64_irq,   default_a64_irq)
    VECTOR_ENTRY(vec_a64_fiq,    vec_lower_el_aarch64_fiq,   default_a64_fiq)
    VECTOR_ENTRY(vec_a64_serror, vec_lower_el_aarch64_serror,default_a64_serror)

    // Lower EL using AArch32
    VECTOR_ENTRY(vec_a32_sync,   vec_lower_el_aarch32_sync,  default_a32_sync)
    VECTOR_ENTRY(vec_a32_irq,    vec_lower_el_aarch32_irq,   default_a32_irq)
    VECTOR_ENTRY(vec_a32_fiq,    vec_lower_el_aarch32_fiq,   default_a32_fiq)
    VECTOR_ENTRY(vec_a32_serror, vec_lower_el_aarch32_serror, default_a32_serror)
);
