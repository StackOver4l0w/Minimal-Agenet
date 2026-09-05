ENTRY(entry)

SECTIONS
{
    /*
     * Code + read-only data
     */
    .text ALIGN(0x1000) :
    {
        __text_start = .;

        *(.text)
        *(.text.*)

        /* Merge read-only data into .text */
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)

        __text_end = .;
    }

    /*
     * Writable initialized data
     */
    .data ALIGN(0x1000) :
    {
        __data_start = .;

        *(.data)
        *(.data.*)

        __data_end = .;
    }

    /*
     * Zero-initialized data
     */
    .bss ALIGN(0x1000) :
    {
        __bss_start = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        __bss_end = .;
    }

    /*
     * Discard compiler/linker metadata that you don't need.
     */
    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame)
        *(.pdata)
        *(.xdata)
    }
}