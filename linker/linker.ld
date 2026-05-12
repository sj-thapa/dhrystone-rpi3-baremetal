ENTRY(init)

SECTIONS
{
    . = 0x00000000;

    /* Code starts with init, then the vector table, then the
     * remainder of the code */
    .text :
    {
        */init.o(boot)
        */vectors.o(vectors)
        *(.text)  /* remainder of code */
    } =0

    /* Data */
    __data_start = .;
    .data : {
        *(.data)
        *(.data.*)    /* All data sections */
    }
    .data1  : { *(.data1) }

    /* Notes section
     * This is not used so we discard it. Although not used it needs to be
     * explicitly mentioned in the linker script as some toolchains will place
     * the notes section at address 0 if it is not explicitly mentioned. */
    /DISCARD/ : { *(.note*) }

    /* Uninitialized data */
    . = ALIGN(8);    /* Align to 8 bytes for AArch64 */
    __bss_start = .;
    __bss_start__ = .;
    .bss : {
        *(.bss)
        *(.bss.*)     /* All BSS sections */
        *(COMMON)     /* Common symbols */
    }
    . = ALIGN(8);
    __bss_end = .;
    __bss_end__ = .;

    . = ALIGN(8);
    __end = .;
    __end__ = .;
}
