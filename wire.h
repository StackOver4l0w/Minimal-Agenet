/* wire.h - tiny little-endian writers for wire formats.
 *
 * Everything on the relay wire is little-endian; these cursor-style helpers
 * append one value at *pos and advance it. Header-only: three functions of
 * three lines each do not justify a .c file.
 */

#pragma once

/* Append a 32-bit value in little-endian byte order. */
static void write_u32_le(unsigned char *buf, int *pos, unsigned value)
{
    for (int i = 0; i < 4; i++)
        buf[(*pos)++] = (unsigned char)(value >> (8 * i));
}

/* Append a 64-bit value in little-endian byte order. */
static void write_u64_le(unsigned char *buf, int *pos,
                         unsigned long long value)
{
    for (int i = 0; i < 8; i++)
        buf[(*pos)++] = (unsigned char)(value >> (8 * i));
}

/* Write an ASCII string into a fixed-width, NUL-padded protocol field.
 * The frame is pre-zeroed by the caller, so only string + NUL are written
 * and the remaining bytes of the field stay zero. */
static void write_ascii_field(unsigned char *buf, int *pos,
                              const char *s, int width)
{
    int start = *pos;
    int i = 0;
    while (s[i] != '\0' && i < width - 1) {
        buf[start + i] = (unsigned char)s[i];
        i++;
    }
    buf[start + i] = '\0';
    *pos = start + width;
}
