#include "types.h"

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...);

#define LOG_INFO(format, ...) PRINT_FORMATTED_STRING("[INF] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) PRINT_FORMATTED_STRING("[ERR] " format "\n", ##__VA_ARGS__)
