/*
    https://github.com/JosefVesely/printfcolor/

    MIT License

    Copyright (c) 2024 Josef Veselý

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef PRINTFCOLOR_H
#define PRINTFCOLOR_H

#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __cplusplus
#include <cstdarg>
extern "C" {
#endif

typedef enum PfcColor {
    GRAY = 0,
    GREY = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7
} PfcColor;

/**
 * Prints formatted output to the console with specified foreground and background colors.
 *
 * Colors: `RED, GREEN, BLUE, WHITE, GRAY (GREY), CYAN, MAGENTA, YELLOW`
 *
 * @param fg_color  The color code for the text foreground color.
 * @param bg_color  The color code for the text background color.
 * @param format    A format string for the output. This is followed by additional arguments
 *                  to be formatted according to the format string.
 *
 * @return  The number of characters written to the console, or `-1` if an error occurs.
 *
 * @warning If the color codes are invalid or if the console screen buffer information cannot
 *          be retrieved, `errno` is set to `EINVAL` and `-1` is returned.
 */
static inline int printfc(PfcColor fg_color, PfcColor bg_color, const char *format, ...);

/**
 * Prints formatted output to the console with specified foreground color.
 *
 * Colors: `RED, GREEN, BLUE, WHITE, GRAY (GREY), CYAN, MAGENTA, YELLOW`
 *
 * @param fg_color  The color code for the text foreground color.
 * @param format    A format string for the output. This is followed by additional arguments
 *                  to be formatted according to the format string.
 *
 * @return  The number of characters written to the console, or `-1` if an error occurs.
 *
 * @warning If the color code is invalid or if the console screen buffer information cannot
 *          be retrieved, `errno` is set to `EINVAL` and `-1` is returned.
 */
static inline int printfc_fg(PfcColor fg_color, const char *format, ...);

static inline int fprintfc_fg(FILE *stream, PfcColor fg_color, const char *format, ...);

static inline int vprintfc_fg(PfcColor fg_color, const char *format, va_list args);

/**
 * Prints formatted output to the console with specified background color.
 *
 * Colors: `RED, GREEN, BLUE, WHITE, GRAY (GREY), CYAN, MAGENTA, YELLOW`
 *
 * @param bg_color  The color code for the text background color.
 * @param format    A format string for the output. This is followed by additional arguments
 *                  to be formatted according to the format string.
 *
 * @return  The number of characters written to the console, or `-1` if an error occurs.
 *
 * @warning If the color code is invalid or if the console screen buffer information cannot
 *          be retrieved, `errno` is set to `EINVAL` and `-1` is returned.
 */
static inline int printfc_bg(PfcColor bg_color, const char *format, ...);

// Helper functions

static bool is_color_valid(PfcColor color) { return color >= 0 && color <= 7; }

static const char *get_fg_color_code(PfcColor color) {
    const char *colors[] = {"\033[30m", "\033[34m", "\033[32m", "\033[36m",
                            "\033[31m", "\033[35m", "\033[33m", "\033[37m"};

    if (is_color_valid(color)) {
        return colors[color];
    } else {
        return NULL;
    }
}

static const char *get_bg_color_code(PfcColor color) {
    const char *colors[] = {"\033[40m", "\033[44m", "\033[42m", "\033[46m",
                            "\033[41m", "\033[45m", "\033[43m", "\033[47m"};

    if (is_color_valid(color)) {
        return colors[color];
    } else {
        return NULL;
    }
}

#ifdef _WIN32 // Windows

static inline int printfc(PfcColor fg_color, PfcColor bg_color, const char *format, ...) {
    if (!is_color_valid(fg_color) || !is_color_valid(bg_color)) {
        errno = EINVAL;
        return -1;
    }

    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_info;

    if (!GetConsoleScreenBufferInfo(h_console, &csbi_info)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    SetConsoleTextAttribute(h_console, fg_color | (bg_color << 4));
    int chars_written = vprintf(format, args);
    SetConsoleTextAttribute(h_console, csbi_info.wAttributes);

    va_end(args);

    return chars_written;
}

static inline int printfc_fg(PfcColor fg_color, const char *format, ...) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_info;

    if (!GetConsoleScreenBufferInfo(h_console, &csbi_info)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    SetConsoleTextAttribute(h_console, fg_color);

    int chars_written = vprintf(format, args);

    SetConsoleTextAttribute(h_console, csbi_info.wAttributes);

    va_end(args);

    return chars_written;
}

static inline int fprintfc_fg(FILE *stream, PfcColor fg_color, const char *format, ...) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    // To nie zadzialaa jak bedzie inny stream niz stdout lub stderr
    HANDLE h_console = GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_info;

    if (!GetConsoleScreenBufferInfo(h_console, &csbi_info)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    SetConsoleTextAttribute(h_console, fg_color);

    int chars_written = vfprintf(stream, format, args);

    SetConsoleTextAttribute(h_console, csbi_info.wAttributes);

    va_end(args);

    return chars_written;
}

static inline int vprintfc_fg(PfcColor fg_color, const char *format, va_list args) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_info;

    if (!GetConsoleScreenBufferInfo(h_console, &csbi_info)) {
        errno = EINVAL;
        return -1;
    }

    SetConsoleTextAttribute(h_console, fg_color);

    int chars_written = vprintf(format, args);

    SetConsoleTextAttribute(h_console, csbi_info.wAttributes);

    return chars_written;
}

static inline int printfc_bg(PfcColor bg_color, const char *format, ...) {
    if (!is_color_valid(bg_color)) {
        errno = EINVAL;
        return -1;
    }

    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi_info;

    if (!GetConsoleScreenBufferInfo(h_console, &csbi_info)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    SetConsoleTextAttribute(h_console, (csbi_info.wAttributes & 0x0F) | (bg_color << 4));
    int chars_written = vprintf(format, args);
    SetConsoleTextAttribute(h_console, csbi_info.wAttributes);

    va_end(args);

    return chars_written;
}

#else // Linux and Unix-like

static inline int printfc(PfcColor fg_color, PfcColor bg_color, const char *format, ...) {
    if (!is_color_valid(fg_color) || !is_color_valid(bg_color)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    printf("%s%s", get_fg_color_code(fg_color), get_bg_color_code(bg_color));
    int chars_written = vprintf(format, args);
    printf("\033[0m");

    va_end(args);

    return chars_written;
}

static inline int printfc_fg(PfcColor fg_color, const char *format, ...) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    printf("%s", get_fg_color_code(fg_color));
    int chars_written = vprintf(format, args);
    printf("\033[0m");

    va_end(args);

    return chars_written;
}

static inline int fprintfc_fg(FILE *stream, PfcColor fg_color, const char *format, ...) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    fprintf(stream, "%s", get_fg_color_code(fg_color));
    int chars_written = vfprintf(stream, format, args);
    fprintf(stream, "\033[0m");

    va_end(args);

    return chars_written;
}

static inline int vprintfc_fg(PfcColor fg_color, const char *format, va_list args) {
    if (!is_color_valid(fg_color)) {
        errno = EINVAL;
        return -1;
    }

    printf("%s", get_fg_color_code(fg_color));
    int chars_written = vprintf(format, args);
    printf("\033[0m");

    return chars_written;
}

static inline int printfc_bg(PfcColor bg_color, const char *format, ...) {
    if (!is_color_valid(bg_color)) {
        errno = EINVAL;
        return -1;
    }

    va_list args;
    va_start(args, format);

    printf("%s", get_bg_color_code(bg_color));
    int chars_written = vprintf(format, args);
    printf("\033[0m");

    va_end(args);

    return chars_written;
}

#endif // _WIN32

#ifdef __cplusplus
}
#endif

#endif // PRINTFCOLOR_H
