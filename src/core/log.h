#ifndef TD_LOG_H
#define TD_LOG_H

#include <stdio.h>

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

extern LogLevel g_log_level;

#define LOG_DEBUG(...) \
    do { \
        if (g_log_level <= LOG_LEVEL_DEBUG) { \
            fprintf(stderr, "[DEBUG] %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } \
    } while (0)

#define LOG_INFO(...) \
    do { \
        if (g_log_level <= LOG_LEVEL_INFO) { \
            fprintf(stderr, "[INFO] %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } \
    } while (0)

#define LOG_WARN(...) \
    do { \
        if (g_log_level <= LOG_LEVEL_WARN) { \
            fprintf(stderr, "[WARN] %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } \
    } while (0)

#define LOG_ERROR(...) \
    do { \
        if (g_log_level <= LOG_LEVEL_ERROR) { \
            fprintf(stderr, "[ERROR] %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
        } \
    } while (0)

#endif /* TD_LOG_H */
