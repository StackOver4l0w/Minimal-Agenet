/* string.c - strings and formatting without libc (see string.h).
 *
 * Three families live here:
 *   - length/compare/convert helpers (strlen, strcmp, AnsiToWide...)
 *   - numeric formatting (int/uint/hex/pointer/double -> chars),
 *     which LOG macros and the identity frame build on;
 *   - Format/FormatV - a printf REPLACEMENT covering exactly the
 *     specifiers this project uses (see FormatV for the list).
 *
 * Formatting writes into caller buffers through an index pointer
 * (*index advances; callers embed several values into one buffer).
 *
 * Recurring theme, see memory.c for the full story: anything the
 * optimizer can recognize as data (float constants, digit tables,
 * sign masks) gets pooled into .rdata. Every such spot below is
 * rewritten as integer arithmetic on volatile locals, so the
 * binary carries the digits inside its instructions instead.
 */

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


// Function to convert an integer to a string with specified formatting - using zero padding, width, and alignment
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

        /*
         * Convert to magnitude without overflowing INT32_MIN.
         */
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

    /*
     * Left alignment disables zero padding.
     */
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

// Function to convert a double to a string with specified formatting - using precision, width, and zero padding
void doubleToStr(double num, PCHAR str, PINT32 index, INT32 precision, INT32 width, INT32 zeroPad) {
    BOOL isNegative = FALSE; // Flag to check if the number is negative

    // Setting negative flag and handling negative numbers
    if (num < 0) {
        isNegative = TRUE;
        str[(*index)++] = '-'; // Add negative sign to the string
        /* Negate by flipping the sign BIT, not by computing -num: the
         * floating-point negation compiles into an SSE xor against a
         * 16-byte constant that would live in .rdata. The union views
         * the same 8 bytes as an integer; flipping bit 63 is the exact
         * IEEE-754 meaning of negation. volatile keeps the shift and
         * the xor as real instructions. */
        union { double d; UINT64 u; } flip;
        volatile UINT64 signbit = 1;
        signbit <<= 63;
        flip.d = num;
        flip.u ^= signbit;
        num = flip.d;
    }

    // Handle the integer part
    INT64 int_part = (INT64)num;
    // Handle the fractional part
    double frac_part = num - int_part;

    // Convert the integer part to string
    INT32 intDigits = 0; // Count the number of digits in the integer part
    INT64 tempInt = int_part; // Temporary variable to count digits in the integer part
    
    // Calculate the number of digits in the integer part
    if (tempInt==0) {
        intDigits = 1;
    } else {
        while (tempInt > 0) {
            tempInt /= 10; // Remove the last digit
            intDigits++; // Increment the digit count
        }
    }

    // Convert the integer part to string
    CHAR intStr[20];  // Temporary storage for integer part
    INT32 intIndex = 0;

    if (int_part==0) {
        intStr[intIndex++] = '0';  // Handle zero case
    } else {
        while (int_part > 0) {
            intStr[intIndex++] = (CHAR)(int_part % 10 + '0'); // Convert last digit to character
            int_part /= 10; // Remove the last digit
        }
    }

    // Reverse the integer part
    for (INT32 i = 0; i < intIndex / 2; ++i) {
        char tmp = intStr[i];
        intStr[i] = intStr[intIndex - 1 - i];
        intStr[intIndex - 1 - i] = tmp;
    }
    
    // Add integer part to the result string
    for (INT32 i = 0; i < intIndex; ++i) {
        str[(*index)++] = intStr[i];
    }

    // Handle the decimal point if precision is specified
    if (precision > 0) {
        str[(*index)++] = '.';  // Add the decimal point
        // Convert the fractional part and round
        /* 10.0f via its IEEE-754 bit pattern: a literal float would be
         * pooled into .rdata by the optimizer; volatile keeps the
         * store from being folded back into one. */
        volatile UINT32 b10 = 0x41200000u;  /* 10.0f */
        float f10 = *(float *)&b10;
        while (precision--) {
            frac_part *= f10;
            INT32 digit = (INT32)frac_part; // Get the integer part of the fractional part
            str[(*index)++] = (CHAR)(digit + '0'); // Convert digit to character
            frac_part -= digit;  // Remove the integer part of the fractional part
        }
        // Round the last digit
        volatile UINT32 b05 = 0x3F000000u;  /* 0.5f */
        float f0_5 = *(float *)&b05;
        if (frac_part >= f0_5) {
            // Increment the last digit
            INT32 last_index = *index - 1; // Get the index of the last digit added
            while (last_index >= 0 && str[last_index]=='9') {
                str[last_index] = '0';
                last_index--;
            }
            if (last_index >= 0) {
                str[last_index]++; // Increment the last digit
            }
            else {
                // Handle overflow (e.g., 999.999 -> 1000.000)
                INT32 carry_index = *index - 1;
                while (carry_index >= 0 && str[carry_index]=='9') {
                    str[carry_index] = '0';
                    carry_index--;
                }
                // If we reached the beginning of the string, we need to add a '1' at the start
                // This handles cases like 999.999 -> 1000.000
                if (carry_index >= 0) {
                    str[carry_index]++;
                }
                else {
                    // If we overflowed the integer part, we need to shift everything
                    // to the right and add '1' at the start
                    str[0] = '1';
                    str[1] = '0';
                }
            }
        }
    }

    // Calculate the total length: integer part + decimal point + precision
    INT32 totalLength = *index - (*index - 1) + precision + (isNegative ? 1 : 0);
    INT32 padding = width - totalLength; // Calculate padding needed to reach the specified width

    // Apply padding if needed
    if (zeroPad) {
        for (INT32 i = 0; i < padding; i++) {
            str[(*index)++] = '0'; // Add zero padding if specified
        }
    } else {
        for (INT32 i = 0; i < padding; i++) { // Add spaces for padding if zero padding is not specified
            str[(*index)++] = ' ';
        }
    }

    // Null-terminate the string
    str[(*index)] = '\0';
}


// Function to convert unsigned integer to string
void uintToStr(UINT64 num, PCHAR str, PINT32 index, INT32 width, INT32 zeroPad, INT32 leftAlign) {
    CHAR rev[20]; // Temporary storage for reversed digits
    INT32 len = 0;  // Length of the number in digits
    INT32 startIndex = *index; // Store the starting index for the string

    // Convert the unsigned number to a reversed string
    do {
        rev[len++] = (num % 10) + '0'; // Convert last digit to character
        num /= 10; // Remove the last digit from the number
    } while (num);

    INT32 totalDigits = len; // Total number of digits in the number
    INT32 paddingSpaces = width - totalDigits; // Calculate padding spaces needed based on width and total digits
    INT32 paddingZeros = 0; // Count of leading zeros to add

    // Calculate padding
    if (zeroPad && !leftAlign) {
        paddingZeros = paddingSpaces > 0 ? paddingSpaces : 0;
        paddingSpaces = 0;
    } else {
        paddingSpaces = paddingSpaces > 0 ? paddingSpaces : 0;
    }

    // If not left-aligned, pad spaces first
    if (!leftAlign) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            str[(*index)++] = ' ';
        }
    }

    // Add leading zeros
    for (INT32 i = 0; i < paddingZeros; ++i) {
        str[(*index)++] = '0';
    }

    // Copy digits in correct order
    while (len) {
        str[(*index)++] = rev[--len];
    }

    // If left-aligned, pad trailing spaces
    if (leftAlign) {
        INT32 printed = *index - startIndex;
        for (INT32 i = printed; i < width; ++i) {
            str[(*index)++] = ' ';
        }
    }
}

// Function to convert pointer address to hexadecimal string
void ptrToHex(PVOID ptr, PCHAR str, PINT32 index) {

    UINT64 addr = (SIZE_T)ptr; // Cast pointer to SIZE to get the address as an unsigned integer
    /* Digits by arithmetic (< 10 ? '0' : 'a'-10), not a lookup
     * table: a table would be initialized data in .rdata. */
    CHAR rev[20]; // Temporary storage for reversed digits
    INT32 len = 0; // Length of the number in digits

    str[(*index)++] = '0'; // Add '0' prefix for hexadecimal representation
    str[(*index)++] = 'x'; // Add 'x' to indicate hexadecimal format

    // Convert the address to a reversed hexadecimal string
    do {
        UINT64 d = addr % 16;
        rev[len++] = (CHAR)(d < 10 ? d + '0' : d - 10 + 'a');
        addr /= 16;
    } while (addr);

    // Copy hexadecimal digits in correct order
    while (len) {
        str[(*index)++] = rev[--len];
    }
}

// Function to convert wide string to narrow string
void wideToStr(PWCHAR wstr, PCHAR str, PINT32 index, INT32 width)
{
    INT32 destIndex = *index; // Start index for the destination string
    INT32 i = 0; // Local index for the wide string
    INT32 len = 0; // Length of the wide string

    // First, calculate length of the wide string
    while (wstr[len] != L'\0') {
        len++;
    }

    // Calculate padding (right-align by default)
    INT32 padding = width - len;
    if (padding < 0) padding = 0;

    // Add padding spaces before the string
    for (INT32 j = 0; j < padding; j++) {
        str[destIndex++] = ' ';
    }

    // Copy each character from the wide string to the narrow string
    while (wstr[i] != L'\0')
    {
        str[destIndex++] = (CHAR)wstr[i];  // Convert wide char to ASCII
        i++;
    }

    // Null-terminate the resulting string.
    str[destIndex] = '\0';

    // Update the original index value.
    *index = destIndex;
}


// Helper function: formats an unsigned number in hexadecimal with a given field width
void formatHex(UINT32 num, INT32 fieldWidth, INT32 uppercase, PCHAR s, INT32* j, INT32 zeroPad, BOOL addPrefix) {
    /* Hex digits computed, not looked up: the compound literals this
     * replaced were const-pooled into .rdata by the optimizer. */
    INT32 base = uppercase ? 'A' : 'a';
    CHAR buffer[16]; // Temporary storage for hexadecimal digits
    INT32 index = 0; // Index for the buffer

    // Special case: if number is zero, we want at least one digit.
    if (num==0) {
        buffer[index++] = '0';
    }
    else {
        while (num) { // Convert number to hexadecimal
            // Get the last digit in hexadecimal and convert it to character
            INT32 d = (INT32)(num % 16);
            buffer[index++] = (CHAR)(d < 10 ? d + '0' : d - 10 + base);
            num /= 16; // Remove the last digit from the number
        }
    }

    // Add prefix (0x or 0X) if needed
    if (addPrefix) {
        s[(*j)++] = '0';
        s[(*j)++] = uppercase ? 'X' : 'x';
    }

    // Calculate padding spaces
    INT32 totalDigits = index + (addPrefix ? 2 : 0);  // Including the prefix
    INT32 paddingSpaces = fieldWidth - totalDigits;
    INT32 paddingZeros = 0;

    // Calculate padding based on zero padding flag
    if (zeroPad) {
        paddingZeros = paddingSpaces > 0 ? paddingSpaces : 0;
        paddingSpaces = 0; // Zero padding overrides space padding
    } else {
        paddingSpaces = paddingSpaces > 0 ? paddingSpaces : 0;
    }

    // Add leading spaces or zeros for right-padding
    if (paddingSpaces > 0) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            s[(*j)++] = ' ';
        }
    }
    
    // Add leading zeros for zero padding
    if (paddingZeros > 0) {
        for (INT32 i = 0; i < paddingZeros; ++i) {
            s[(*j)++] = '0';
        }
    }

    // Copy digits in correct order (reverse the buffer into the string)
    while (index) {
        s[(*j)++] = buffer[--index];
    }

    // Add trailing spaces for left-padding (if any)
    if (!zeroPad && paddingSpaces > 0) {
        for (INT32 i = 0; i < paddingSpaces; ++i) {
            s[(*j)++] = ' ';
        }
    }
}


// Custom vsprintf function implementation
INT32 FormatV(PCHAR s, PCHAR format, va_list args) {
 
    INT32 i = 0, j = 0; // Index for the format string and output string
    INT32 precision = 6;  // Default precision for floating-point numbers

    // Validate the output string
    if (format==NULL) {
        return 0;
    }
 
    // Loop through the format string to process each character
    while (format[i] != '\0') {
        if (format[i]=='%') {
            i++;  // Skip '%'
            precision = 6;  // Reset default precision

            // Handle precision for floating-point numbers (e.g. "%.3f")
            if (format[i]=='.') {
                i++;  // Skip '.'
                precision = 0; // Reset precision to 0 before parsing
                while (format[i] >= '0' && format[i] <= '9') { // Parse precision value
                    precision = precision * 10 + (format[i] - '0'); // Convert character to integer
                    i++; // Move to the next character
                }
            }

            INT32 addPrefix = 0;  // Flag for adding '0x' prefix to hex numbers
            if (format[i]=='#') {
                addPrefix = 1;  // Set flag to add prefix
                i++;  // Skip '#'
            }
            // Check for an optional zero flag and field width
            INT32 leftAlign = 0;
            INT32 zeroPad = 0;
            INT32 fieldWidth = 0;
            
            // Check for optional flags: '-' for left align, '0' for zero padding
            while (format[i]=='-' || format[i]=='0') {
                if (format[i]=='-') {
                    leftAlign = 1; // Set left alignment flag
                    zeroPad = 0;  // '-' overrides '0'
                } else if (format[i]=='0' && !leftAlign) {
                    zeroPad = 1; // Set zero padding flag only if not left aligned
                }
                i++;
            }
            
            // Parse any numeric field width
            while (format[i] >= '0' && format[i] <= '9') {
                fieldWidth = fieldWidth * 10 + (format[i] - '0');
                i++;
            }

            // Now switch based on the conversion specifier
            if (format[i]=='X') {
                i++;  // Skip 'X'
                UINT32 num = (UINT32)va_arg(args, UINT32);
                // Format the number as uppercase hexadecimal.
                formatHex(num, fieldWidth, 1, s, &j, zeroPad, addPrefix);

                // If a '-' follows, add it (for MAC address separators)
                if (format[i]=='-') {
                    s[j++] = '-';
                    i++;  // Skip the hyphen
                }
                continue;
            }
             // NOTE: making specifiers lowercase to handle both cases (e.g., %d and %D), that's why we use TO_LOWER_CASE macro
            else if (TO_LOWER_CASE(format[i])=='d') {  // Handle %d (signed integer) : 
                INT32 num = va_arg(args, INT32); // Get the next argument as an INT32
                intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the integer to string with specified formatting
                i++; // Skip 'd'
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='u') {  // Handle %u (unsigned integer)
                UINT32 num = va_arg(args, UINT32); // Get the next argument as an UINT32
                uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the unsigned integer to string with specified formatting
                i++; // Skip 'u'
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='x') {  // Handle %x (hexadecimal, lowercase)
                i++;  // Skip 'x'
                UINT32 num = va_arg(args, UINT32); // Get the next argument as an UINT32
                formatHex(num, fieldWidth, 0, s, &j, zeroPad, addPrefix); // Convert the number to lowercase hexadecimal with specified formatting
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='p') {  // Handle %p (pointer)
                i++; // Skip 'p'
                ptrToHex(va_arg(args, PVOID), s, &j); // Convert the pointer address to hexadecimal string
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='c') {  // Handle %c (character)
                // Loop through the field width to ensure proper spacing
                for (INT32 k = 0; k < fieldWidth - 1; k++) {
                    s[j++] = ' '; // Add spaces for field width
                }
                s[j++] = (CHAR)va_arg(args, INT32); // Get the next argument as an INT32 (character) and add it to the string
                i++; // Skip 'c'
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='s' ) {  // Handle %s (narrow string)
                i++; // Skip 's'
                PCHAR str = va_arg(args, PCHAR); // Get the next argument as a PCHAR (narrow string)
                // C standard does not allow NULL strings, so if the string is NULL, handle it by printing "(null)".
                if(str==NULL) { 
                    str = ((CHAR[]){'(','n','u','l','l',')','\0'});  // Handle null string case
                }
                INT32 len = 0; // Length of the string to be printed

                // Checking the string is not NULL and calculating its length
                if (str) {
                    PCHAR temp = str;
                    while (*temp) {
                        len++;
                        temp++;
                    }
                    INT32 padding = fieldWidth - len; // Calculate padding based on field width and string length
                    if (padding < 0) padding = 0; // Ensure padding is non-negative

                    // If left-aligned, copy the string directly
                    for (int k = 0; k < padding; k++) {
                        s[j++] = ' ';
                    }
                    // Copy the string to the output
                    while (*str) {
                        s[j++] = *str++; // Copy each character from the string to the output
                    }
                }
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='w') {  // Handle %ws (wide string)
                if (TO_LOWER_CASE(format[i+1])=='s') {
                    i += 2; // Skip over "ws"
                    PWCHAR wstr = va_arg(args, PWCHAR); // Get the next argument as a PWCHAR (wide string)
                    // C standard does not allow NULL strings, so if the string is NULL, handle it by printing "(null)".
                    if(wstr==NULL) {
                        wstr = ((WCHAR[]){L'(',L'n',L'u',L'l',L'l',L')',L'\0'});  // Handle null wide string case
                    }
                    wideToStr(wstr, s, &j, fieldWidth); // Convert the wide string to narrow string with specified formatting
                    continue;
                }
                else {
                    s[j++] = format[i++]; // If it's not %ws, just copy the character as is.
                    continue;
                }
            }
            // Support %ls (wide string) in the same way as %ws
            else if (TO_LOWER_CASE(format[i])=='l') {
                if (TO_LOWER_CASE(format[i+1])=='s') {
                    i += 2; // Skip over "ls"
                    PWCHAR wstr = va_arg(args, PWCHAR); // Get the next argument as a PWCHAR (wide string)
                    // C standard does not allow NULL strings, so if the string is NULL, handle it by printing "(null)".
                    if(wstr==NULL) {
                        wstr = ((WCHAR[]){L'(',L'n',L'u',L'l',L'l',L')',L'\0'});  // Handle null wide string case
                    }
                    wideToStr(wstr, s, &j, fieldWidth); // Convert the wide string to narrow string with specified formatting
                    continue;
                }
                // Handle other long variants (lf, ld, lu, lld)
                else if (TO_LOWER_CASE(format[i+1])=='f') {  // long double (%lf)
                    i += 2; // Skip over "lf"
                    long double num = va_arg(args, long double); // Get the next argument as a long double
                    doubleToStr(num, s, &j, precision, fieldWidth, zeroPad); // Convert the long double to string with specified formatting
                    continue;
                }
                else if (TO_LOWER_CASE(format[i+1])=='d') {  // long int (%ld)
                    i += 2; // Skip over "ld"
                    INT32 num = va_arg(args, INT32); // Get the next argument as an INT32 (long int)
                    intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the long int to string with specified formatting
                    continue;
                }
                else if (TO_LOWER_CASE(format[i+1])=='u') {  // unsigned long int (%lu)
                    i += 2; // Skip over "lu"
                    UINT32 num = va_arg(args, UINT32); // Get the next argument as an UINT32 (unsigned long int)
                    uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the unsigned long int to string with specified formatting
                    continue;
                }
                else if (TO_LOWER_CASE(format[i + 1])=='l' && TO_LOWER_CASE(format[i + 2])=='d') {  // long long int (%lld)
                    i += 3; // Skip over "lld"
                    INT64 num = va_arg(args, INT64); // Get the next argument as an INT64 (long long int)
                    intToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the long long int to string with specified formatting
                    continue;
                }
                else if(TO_LOWER_CASE(format[i+1])=='l' && TO_LOWER_CASE(format[i+2])=='u'){
                    i += 3; // Skip over "llu"
                    UINT64 num = va_arg(args, UINT64); // Get the next argument as an UINT64 (unsigned long long int)
                    uintToStr(num, s, &j, fieldWidth, zeroPad, leftAlign); // Convert the unsigned long long int to string with specified formatting
                    continue;
                }
                else {
                    s[j++] = format[i++]; // If it's not recognized, just copy the character as is.
                    continue;
                }
            }
            else if (TO_LOWER_CASE(format[i])=='f') {  // Handle %f (double)
                i++; // Skip 'f'
                double num = va_arg(args, double); // Get the next argument as a double
                doubleToStr(num, s, &j, precision, fieldWidth, zeroPad); // Convert the double to string with specified formatting
                continue;
            }
            else if (TO_LOWER_CASE(format[i])=='%') { // Handle literal "%%"
                s[j++] = '%'; // Output a literal '%'
                i++; // Skip the '%'
                continue;
            }
            else {  // Unknown specifier: output it as-is.
                s[j++] = format[i++]; // Copy the unknown specifier character to the output string
                continue;
            }
        }
        else {  // Ordinary character: copy it.
            s[j++] = format[i++];
        }
    }
    s[j] = '\0'; // Null-terminate the output string
    return j; // Return the length of the formatted string
}

// Function to format a string with variable arguments
INT32 Format(PCHAR s, PCHAR format, ...) {
    va_list args; // Variatic arguments list
    va_start(args, format); // Initialize the argument list with format so we can access the variable arguments
    INT32 len = FormatV(s, format, args); // Format the string using the variable arguments
    va_end(args); // Clean up the argument list
    return len; // Return the length of the formatted string
}