/* link.text-first.ld - single .text, entry() at byte zero.
 *
 * WHY: the deliverable form of this agent is a raw .text blob; whatever
 * loads it jumps to byte 0. entry() is emitted directly into .text and
 * entry.c is linked first, so it becomes the first byte of the output.
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
        *(.text)
        *(.text.*)
    }
    /DISCARD/ : { *(.comment) }
}
