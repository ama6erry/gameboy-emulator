#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "utils.h"


void base_log(LogLevel level, const char *file, int line, const char *format, ...) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local);

    const char *level_strings[] = {"INFO", "WARN", "ERROR"};

    printf("[%s] [%s] (%s:%d) ", time_str, level_strings[level], file, line);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n"); 
}



