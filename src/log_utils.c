#include "printfcolor.h"
#include <stdio.h>

void verbose(char *format, ...) {
    va_list args;
    printfc_fg(GREY, "VRBOS: ");
    va_start(args, format);
    vprintfc_fg(GREY, format, args);
    va_end(args);
}

void info(char *format, ...) {
    va_list args;
    printfc_fg(BLUE, "INFO : ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void warn(char *format, ...) {
    va_list args;
    fprintfc_fg(stderr, YELLOW, "WARN : ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

void error(char *format, ...) {
    va_list args;
    fprintfc_fg(stderr, RED, "ERROR: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
