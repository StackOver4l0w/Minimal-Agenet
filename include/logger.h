/* logger.h - terminal logging as a BUILD-TIME OPTION (see logger.c).
 *
 * A dev build passes -DLOGGING_ENABLED and gets [INF]/[ERR] lines on
 * stdout. A release build defines nothing: every LOG_* expands to
 * whitespace, and not one format string enters the binary. A quiet
 * agent cannot be told apart from any other quiet tool - that is the
 * point of the switch.
 */

#include "types.h"

INT32 PRINT_FORMATTED_STRING(PCHAR format, ...);

#ifdef LOGGING_ENABLED
#define LOG_INFO(format, ...) PRINT_FORMATTED_STRING("[INF] " format "\n", ##__VA_ARGS__)
#define LOG_ERROR(format, ...) PRINT_FORMATTED_STRING("[ERR] " format "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#define LOG_ERROR(format, ...)
#endif
