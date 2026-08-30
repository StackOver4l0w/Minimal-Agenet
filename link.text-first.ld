/* link.text-first.ld - single .text, entry() at byte zero.
 *
 * WHY: the deliverable form of this agent is a raw .text blob; whatever
 * loads it jumps to byte 0. The default mingw layout puts plain .text
 * before .text.startup (where entry() lives at offset 0 of its object),
 * pushing entry ~15 KB deep. This script merges all code into ONE
 * output section and lists .text.startup FIRST, so entry becomes the
 * first byte of the final binary.
 *
 * Side effect: no .rdata/.idata are created at all - the link itself
 * now produces the single-section PE (the objcopy surgery step became
 * unnecessary).
 *
 * Used via: gcc ... -nostdlib -e entry -Wl,-T,link.text-first.ld
 */
SECTIONS
{
    .text 0x140001000 :
    {
        *(.text.startup)
        *(.text)
        *(.text.*)
    }
    /DISCARD/ : { *(.comment) }
}
