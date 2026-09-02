#include "string.h"
#include "types.h"

#define TO_LOWER_CASE(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + ('a' - 'A') : (c))

__SIZE_TYPE__ strlen(const CHAR *s) {
    SIZE_T len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

__SIZE_TYPE__ wcslen(const WCHAR *s) {
    SIZE_T len = 0;
    while (s[len] != L'\0') {
        len++;
    }
    return len;
}

__SIZE_TYPE__ strlen_w(const WCHAR *s) {
    return wcslen(s);
}

INT32 AnsiToWide(const CHAR *ansi, PWCHAR wide, INT32 wideSize) {
    if (ansi == NULL || wide == NULL || wideSize <= 0) {
        return -1;
    }

    INT32 i = 0;
    for (; i < wideSize - 1 && ansi[i] != '\0'; ++i) {
        wide[i] = (WCHAR)ansi[i];
    }
    wide[i] = L'\0';

    return i;
}

void intToStr(INT32 num, PCHAR str, PINT32 index,
              INT32 width, INT32 zeroPad, INT32 leftAlign)
{
    CHAR rev[16];
    INT32 len = 0;
    INT32 startIndex = *index;
    BOOL negative = FALSE;
    UINT32 magnitude;

    if (num < 0) {
        negative = TRUE;

        magnitude = (UINT32)(-(num + 1));
        magnitude += 1;
    } else {
        magnitude = (UINT32)num;
    }

    do {
        rev[len++] = (CHAR)('0' + (magnitude % 10));
        magnitude /= 10;
    } while (magnitude != 0);

    INT32 contentWidth = len + (negative ? 1 : 0);
    INT32 padding = width - contentWidth;

    if (padding < 0) {
        padding = 0;
    }

    if (!leftAlign && zeroPad) {

        if (negative) {
            str[(*index)++] = '-';
        }

        while (padding-- > 0) {
            str[(*index)++] = '0';
        }

    } else {

        if (!leftAlign) {
            while (padding-- > 0) {
                str[(*index)++] = ' ';
            }
        }

        if (negative) {
            str[(*index)++] = '-';
        }
    }

    while (len > 0) {
        str[(*index)++] = rev[--len];
    }

    if (leftAlign) {
        INT32 printed = *index - startIndex;

        while (printed < width) {
            str[(*index)++] = ' ';
            printed++;
        }
    }
}

void doubleToStr(double num, PCHAR str, PINT32 index, INT32 precision, INT32 width, INT32 zeroPad) {
    BOOL isNegative = FALSE;

    if (num < 0) {
        isNegative = TRUE;
        str[(*index)++] = '-';

        union { double d; UINT64 u; } flip;
        volatile UINT64 signbit = 1;
        signbit <<= 63;
        flip.d = num;
        flip.u ^= signbit;
        num = flip.d;
    }

    INT64 int_part = (INT64)num;

    double frac_part = num - int_part;

    INT32 intDigits = 0;
    INT64 tempInt = int_part;

    if (tempInt==0) {
        intDigits = 1;
    } else {
        while (tempInt > 0) {
            tempInt /= 10;
            intDigits++;
        }
    }

    CHAR intStr[20];
    INT32 intIndex = 0;

    if (int_part==0) {
        intStr[intIndex++] = '0';
    } else {
        while (int_part > 0) {
            intStr[intIndex++] = (CHAR)(int_part % 10 + '0');
            int_part /= 10;
        }
    }

    for (INT32 i = 0; i < intIndex / 2; ++i) {
        char tmp = intStr[i];
        intStr[i] = intStr[intIndex - 1 - i];
        intStr[intIndex - 1 - i] = tmp;
    }

    for (INT32 i = 0; i < intIndex; ++i) {
        str[(*index)++] = intStr[i];
    }

    if (precision > 0) {
        str[(*index)++] = '.';

        volatile UINT32 b10 = 0x41200000u;
        float f10 = *(float *)&b10;
        while (precision--) {
            frac_part *= f10;
            INT32 digit = (INT32)frac_part;
            str[(*index)++] = (CHAR)(digit + '0');
            frac_part -= digit;
        }

        volatile UINT32 b05 = 0x3F000000u;
        float f0_5 = *(float *)&b05;
        if (frac_part >= f0_5) {

            INT32 last_index = *index - 1;
            while (last_index >= 0 && str[last_index]=='9') {
                str[last_index] = '0';
                last_index--;
            }
            if (last_index >= 0) {
                str[last_index]++;
            }
            else {

                INT32 carry_index = *index - 1;
                while (carry_index >= 0 && str[carry_index]=='9') {
                    str[carry_index] = '0';
                    carry_index--;
                }

                if (carry_index >= 0) {
                    str[carry_index]++;
                }
                else {

                    str[0] = '1';
                    str[1] = '0';
                }
            }
        }
    }

    INT32 totalLength = *index - (*index - 1) + precision + (isNegative ? 1 : 0);
    INT32 padding = width - totalLength;

    if (zeroPad) {
        for (INT32 i = 0; i < padding; i++) {
            str[(*index)++] = '0';
        }
    } else {
        for (INT32 i = 0; i < padding; i++) {
            str[(*index)++] = ' ';
        }
    }

    str[(*index)] = '\0';
}

void uintToStr(UINT64 num, PCHAR str, PINT32 index, INT32 width, INT32 zeroPad, INT32 leftAlign) {
    CHAR rev[20];
    INT32 len = 0;
    INT32 startIndex = *index;

    do {
        rev[len++] = (num % 10) + '0';
        num /= 10;
    } while (num);

    INT32 totalDigits = len;
    INT32 paddingSpaces = width - totalDigits;
    INT32 paddingZeros = 0;

    if (zeroPad && !leftAlign) {
        paddingZeros = paddingSpaces > 0 ? paddingSpaces : 0;
        paddingSpaces = 0;
    } else {
        paddingSpaces = paddingSpaces > 0 ? paddingSpaces : 0;
    }

    if (!leftAlign) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            str[(*index)++] = ' ';
        }
    }

    for (INT32 i = 0; i < paddingZeros; ++i) {
        str[(*index)++] = '0';
    }

    while (len) {
        str[(*index)++] = rev[--len];
    }

    if (leftAlign) {
        INT32 printed = *index - startIndex;
        for (INT32 i = printed; i < width; ++i) {
            str[(*index)++] = ' ';
        }
    }
}

void ptrToHex(PVOID ptr, PCHAR str, PINT32 index) {

    UINT64 addr = (SIZE_T)ptr;

    CHAR rev[20];
    INT32 len = 0;

    str[(*index)++] = '0';
    str[(*index)++] = 'x';

    do {
        UINT64 d = addr % 16;
        rev[len++] = (CHAR)(d < 10 ? d + '0' : d - 10 + 'a');
        addr /= 16;
    } while (addr);

    while (len) {
        str[(*index)++] = rev[--len];
    }
}

void wideToStr(PWCHAR wstr, PCHAR str, PINT32 index, INT32 width)
{
    INT32 destIndex = *index;
    INT32 i = 0;
    INT32 len = 0;

    while (wstr[len] != L'\0') {
        len++;
    }

    INT32 padding = width - len;
    if (padding < 0) padding = 0;

    for (INT32 j = 0; j < padding; j++) {
        str[destIndex++] = ' ';
    }

    while (wstr[i] != L'\0')
    {
        str[destIndex++] = (CHAR)wstr[i];
        i++;
    }

    str[destIndex] = '\0';

    *index = destIndex;
}

void formatHex(UINT32 num, INT32 fieldWidth, INT32 uppercase, PCHAR s, INT32* j, INT32 zeroPad, BOOL addPrefix) {

    INT32 base = uppercase ? 'A' : 'a';
    CHAR buffer[16];
    INT32 index = 0;

    if (num==0) {
        buffer[index++] = '0';
    }
    else {
        while (num) {

            INT32 d = (INT32)(num % 16);
            buffer[index++] = (CHAR)(d < 10 ? d + '0' : d - 10 + base);
            num /= 16;
        }
    }

    if (addPrefix) {
        s[(*j)++] = '0';
        s[(*j)++] = uppercase ? 'X' : 'x';
    }

    INT32 totalDigits = index + (addPrefix ? 2 : 0);
    INT32 paddingSpaces = fieldWidth - totalDigits;
    INT32 paddingZeros = 0;

    if (zeroPad) {
        paddingZeros = paddingSpaces > 0 ? paddingSpaces : 0;
        paddingSpaces = 0;
    } else {
        paddingSpaces = paddingSpaces > 0 ? paddingSpaces : 0;
    }

    if (paddingSpaces > 0) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            s[(*j)++] = ' ';
        }
    }

    if (paddingZeros > 0) {
        for (INT32 i = 0; i < paddingZeros; ++i) {
            s[(*j)++] = '0';
        }
    }

    while (index) {
        s[(*j)++] = buffer[--index];
    }

    if (!zeroPad && paddingSpaces > 0) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            s[(*j)++] = ' ';
        }
    }
}

INT32 FormatV(PCHAR s, PCHAR format, va_list args) {

    INT32 i = 0, j = 0;
    INT32 precision = 6;

    if (format==NULL) {
        return 0;
    }

    while (format[i] != '\0') {
        if (format[i]=='%') {
            i++;
            precision = 6;

            if (format[i]=='.') {
                i++;
                precision = 0;
                while (format[i] >= '0' && format[i] <= '9') {
                    precision = precision * 10 + (format[i] - '0');
                    i++;
                }
            }

            INT32 addPrefix = 0;
            if (format[i]=='#') {
                addPrefix = 1;
                i++;
            }

            INT32 leftAlign = 0;
            INT32 zeroPad = 0;
            INT32 fieldWidth = 0;

            while (format[i]=='-' || format[i]=='0') {
                if (format[i]=='-') {
                    leftAlign = 1;
                    zeroPad = 0;
                } else if (format[i]=='0' && !leftAlign) {
                    zeroPad = 1;
                }
                i++;
            }

            while (format[i] >= '0' && format[i] <= '9') {
                fieldWidth = fieldWidth * 10 + (format[i] - '0');
                i++;
            }

            if (format[i]=='X') {
                i++;
                UINT32 num = (UINT32)va_arg(args, UINT32);

                formatHex(num, fieldWidth, 1, s, &j, zeroPad, addPrefix);

                if (format[i]=='-') {
                    s[j++] = '-';
                    i++;
                }
                continue;
            }

            else if (TO_LOWER_CASE(format[i])=='d') {
                INT32 num = va_arg(args, INT32);
                intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                i++;
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='u') {
                UINT32 num = va_arg(args, UINT32);
                uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                i++;
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='x') {
                i++;
                UINT32 num = va_arg(args, UINT32);
                formatHex(num, fieldWidth, 0, s, &j, zeroPad, addPrefix);
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='p') {
                i++;
                ptrToHex(va_arg(args, PVOID), s, &j);
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='c') {

                for (INT32 k = 0; k < fieldWidth - 1; k++) {
                    s[j++] = ' ';
                }
                s[j++] = (CHAR)va_arg(args, INT32);
                i++;
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='s' ) {
                i++;
                PCHAR str = va_arg(args, PCHAR);

                if(str==NULL) {
                    str = ((CHAR[]){'(','n','u','l','l',')','\0'});
                }
                INT32 len = 0;

                if (str) {
                    PCHAR temp = str;
                    while (*temp) {
                        len++;
                        temp++;
                    }
                    INT32 padding = fieldWidth - len;
                    if (padding < 0) padding = 0;

                    for (int k = 0; k < padding; k++) {
                        s[j++] = ' ';
                    }

                    while (*str) {
                        s[j++] = *str++;
                    }
                }
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='w') {
                if (TO_LOWER_CASE(format[i+1])=='s') {
                    i += 2;
                    PWCHAR wstr = va_arg(args, PWCHAR);

                    if(wstr==NULL) {
                        wstr = ((WCHAR[]){L'(',L'n',L'u',L'l',L'l',L')',L'\0'});
                    }
                    wideToStr(wstr, s, &j, fieldWidth);
                    continue;
                }
                else {
                    s[j++] = format[i++];
                    continue;
                }
            }

            else if (TO_LOWER_CASE(format[i])=='l') {
                if (TO_LOWER_CASE(format[i+1])=='s') {
                    i += 2;
                    PWCHAR wstr = va_arg(args, PWCHAR);

                    if(wstr==NULL) {
                        wstr = ((WCHAR[]){L'(',L'n',L'u',L'l',L'l',L')',L'\0'});
                    }
                    wideToStr(wstr, s, &j, fieldWidth);
                    continue;
                }

                else if (TO_LOWER_CASE(format[i+1])=='f') {
                    i += 2;
                    long double num = va_arg(args, long double);
                    doubleToStr(num, s, &j, precision, fieldWidth, zeroPad);
                    continue;
                }
                else if (TO_LOWER_CASE(format[i+1])=='d') {
                    i += 2;
                    INT32 num = va_arg(args, INT32);
                    intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                    continue;
                }
                else if (TO_LOWER_CASE(format[i+1])=='u') {
                    i += 2;
                    UINT32 num = va_arg(args, UINT32);
                    uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                    continue;
                }
                else if (TO_LOWER_CASE(format[i + 1])=='l' && TO_LOWER_CASE(format[i + 2])=='d') {
                    i += 3;
                    INT64 num = va_arg(args, INT64);
                    intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                    continue;
                }
                else if(TO_LOWER_CASE(format[i+1])=='l' && TO_LOWER_CASE(format[i+2])=='u'){
                    i += 3;
                    UINT64 num = va_arg(args, UINT64);
                    uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign);
                    continue;
                }
                else {
                    s[j++] = format[i++];
                    continue;
                }
            }
            else if (TO_LOWER_CASE(format[i])=='f') {
                i++;
                double num = va_arg(args, double);
                doubleToStr(num, s, &j, precision, fieldWidth, zeroPad);
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='%') {
                s[j++] = '%';
                i++;
                continue;
            }
            else {
                s[j++] = format[i++];
                continue;
            }
        }
        else {
            s[j++] = format[i++];
        }
    }
    s[j] = '\0';
    return j;
}

INT32 Format(PCHAR s, PCHAR format, ...) {
    va_list args;
    va_start(args, format);
    INT32 len = FormatV(s, format, args);
    va_end(args);
    return len;
}
