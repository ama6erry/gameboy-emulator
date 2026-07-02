#include <stdio.h>

typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

void base_log(LogLevel level, const char *file, int line, const char *format, ...);

#define LOG_I(fmt, ...) base_log(LOG_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) base_log(LOG_WARN,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) base_log(LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)