#include "types.h"

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...);

/* C1 step 3a: logging is a DEV tool, not a release feature. It is OFF
 * by default; build with -DLOGGING_ENABLED to get the terminal logs.
 * Every log format string lives in .rdata - a release build must be
 * silent so the section can die. */
#ifdef LOGGING_ENABLED
#define LOG_INFO(format, ...) PRINT_FORMATTED_STRING("[INF] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) PRINT_FORMATTED_STRING("[ERR] " format "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#define LOG_ERROR(format, ...)
#endif