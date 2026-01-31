#ifndef EOS_LAN_LOGGING_H
#define EOS_LAN_LOGGING_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} LogLevel;

/**
 * Initialize logging system.
 * @param level Minimum level to log
 * @param file Optional file to write to (NULL = stderr only)
 */
void log_init(LogLevel level, const char* file);

/**
 * Shutdown logging system.
 */
void log_shutdown(void);

/**
 * Set log level at runtime.
 */
void log_set_level(LogLevel level);

/**
 * Log a message.
 */
void log_write(LogLevel level, const char* func, const char* fmt, ...);

// Convenience macros
#define EOS_LOG_ERROR(fmt, ...) log_write(LOG_LEVEL_ERROR, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_WARN(fmt, ...)  log_write(LOG_LEVEL_WARN, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_INFO(fmt, ...)  log_write(LOG_LEVEL_INFO, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_DEBUG(fmt, ...) log_write(LOG_LEVEL_DEBUG, __func__, fmt, ##__VA_ARGS__)
#define EOS_LOG_TRACE(fmt, ...) log_write(LOG_LEVEL_TRACE, __func__, fmt, ##__VA_ARGS__)

// API call logging (logs function entry/exit with params)
#define EOS_LOG_API_ENTER() EOS_LOG_TRACE("ENTER")
#define EOS_LOG_API_RETURN(result) do { \
    EOS_LOG_TRACE("RETURN %s", EOS_EResult_ToString(result)); \
    return result; \
} while(0)

#endif // EOS_LAN_LOGGING_H
