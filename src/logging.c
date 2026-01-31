#include "internal/logging.h"
#include <time.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
static CRITICAL_SECTION g_log_mutex;
static BOOL g_log_mutex_initialized = FALSE;
#else
#include <pthread.h>
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static LogLevel g_log_level = LOG_LEVEL_INFO;
static FILE* g_log_file = NULL;

static const char* level_names[] = {
    "NONE", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

void log_init(LogLevel level, const char* file) {
    g_log_level = level;

#ifdef _WIN32
    if (!g_log_mutex_initialized) {
        InitializeCriticalSection(&g_log_mutex);
        g_log_mutex_initialized = TRUE;
    }
#endif

    if (file && strlen(file) > 0) {
        g_log_file = fopen(file, "a");
        if (!g_log_file) {
            fprintf(stderr, "[EOSLAN] Failed to open log file: %s (errno: %d)\n", file, errno);
        } else {
            fprintf(stderr, "[EOSLAN] Log file opened: %s\n", file);
        }
    }
}

void log_shutdown(void) {
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }

#ifdef _WIN32
    if (g_log_mutex_initialized) {
        DeleteCriticalSection(&g_log_mutex);
        g_log_mutex_initialized = FALSE;
    }
#endif
}

void log_set_level(LogLevel level) {
    g_log_level = level;
}

void log_write(LogLevel level, const char* func, const char* fmt, ...) {
    if (level > g_log_level) return;

#ifdef _WIN32
    if (g_log_mutex_initialized) {
        EnterCriticalSection(&g_log_mutex);
    }
#else
    pthread_mutex_lock(&g_log_mutex);
#endif

    // Get timestamp
    time_t now = time(NULL);
    struct tm* tm = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", tm);

    // Format message
    char message[2048];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    // Output format: [TIME] [LEVEL] [FUNC] message
    char output[2200];
    snprintf(output, sizeof(output), "[%s] [%-5s] [%s] %s\n",
             timestamp, level_names[level], func, message);

    // Write to stderr
    fputs(output, stderr);

    // Write to file if configured
    if (g_log_file) {
        fputs(output, g_log_file);
        fflush(g_log_file);
    }

#ifdef _WIN32
    if (g_log_mutex_initialized) {
        LeaveCriticalSection(&g_log_mutex);
    }
#else
    pthread_mutex_unlock(&g_log_mutex);
#endif
}
