#include "types.h"

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...);

#define LOGGING_ENABLED

#ifdef  LOGGING_ENABLED
#define LOG_INFO(format, ...) PRINT_FORMATTED_STRING("[INF] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) PRINT_FORMATTED_STRING("[ERR] " format "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#define LOG_ERROR(format, ...)
#endif