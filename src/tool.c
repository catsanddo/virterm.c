#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STREQ(a,b) (strcmp(a, b) == 0)

#ifdef _WIN32
#include <wchar.h>
#include <windows.h>

DWORD dwOriginalOutMode = 0;
DWORD dwOriginalInMode = 0;

int init(void)
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    dwOriginalOutMode = 0;
    dwOriginalInMode = 0;
    if (!GetConsoleMode(hOut, &dwOriginalOutMode))
    {
        return 0;
    }
    if (!GetConsoleMode(hIn, &dwOriginalInMode))
    {
        return 0;
    }

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if (!SetConsoleMode(hOut, dwOutMode))
    {
        // we failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
        if (!SetConsoleMode(hOut, dwOutMode))
        {
            // Failed to set any VT mode, can't do anything here.
            return 0;
        }
    }

    DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
    if (!SetConsoleMode(hIn, dwInMode))
    {
        // Failed to set VT input mode, can't do anything here.
        return 0;
    }

    return 1;
}

void deinit(void)
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SetConsoleMode(hOut, dwOriginalOutMode);
    SetConsoleMode(hIn, dwOriginalInMode);
}
#else
int init(void)
{
    return 1;
}

void deinit(void)
{
}
#endif

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#define BRIGHT_BLACK 8
#define BRIGHT_RED 9
#define BRIGHT_GREEN 10
#define BRIGHT_YELLOW 11
#define BRIGHT_BLUE 12
#define BRIGHT_MAGENTA 13
#define BRIGHT_CYAN 14
#define BRIGHT_WHITE 15

void reset_mode(void)
{
    printf("\x1b[0m");
}

void clear(void)
{
    printf("\x1b[2J");
}

char *shift(char **argv)
{
    if (argv == 0 || argv[0] == 0) {
        return 0;
    }

    char *arg0 = argv[0];

    int i = 0;
    while (argv[i]) {
        argv[i] = argv[i+1];
        i += 1;
    }

    return arg0;
}

int is_number(const char *s)
{
    while (*s != 0) {
        if ('0' > *s || *s > '9') {
            return 0;
        }
        s++;
    }

    return 1;
}

int main(int argc, char **argv)
{
    if (!init()) {
        printf("Couldn't init.\n");
        goto error;
    }

    shift(argv);

    char *command;
    while ((command = shift(argv)) != 0) {
        if (STREQ(command, "reset")) {
            reset_mode();
        } else if (STREQ(command, "clear")) {
            clear();
        } else if (STREQ(command, "move")) {
            char *row = shift(argv);
            char *col = shift(argv);

            if (row == 0 || col == 0) {
                printf("Please provide a row AND column for %s\n", command);
                goto error;
            }
            if (!is_number(row)) {
                printf("Invalid number '%s' for <row>.\n", row);
                goto error;
            } else if (!is_number(col)) {
                printf("Invalid number '%s' for <column>.\n", col);
                goto error;
            }

            printf("\x1b[%s;%sH", row, col);
        } else if (STREQ(command, "set")) {
            char *attribute = shift(argv);

            int attr;
            if (STREQ(attribute, "bold")) {
                attr = 1;
            } else if (STREQ(attribute, "dim")) {
                attr = 2;
            } else if (STREQ(attribute, "italic")) {
                attr = 3;
            } else if (STREQ(attribute, "underline")) {
                attr = 4;
            } else if (STREQ(attribute, "blink")) {
                attr = 5;
            } else if (STREQ(attribute, "reverse")) {
                attr = 7;
            } else if (STREQ(attribute, "invisible")) {
                attr = 8;
            } else if (STREQ(attribute, "strike")) {
                attr = 9;
            } else {
                printf("Unrecognized attribute '%s'\n", attribute);
                goto error;
            }
            printf("\x1b[%dm", attr);
        } else if (STREQ(command, "unset")) {
            char *attribute = shift(argv);

            int attr;
            if (STREQ(attribute, "bold")) {
                attr = 2;
            } else if (STREQ(attribute, "dim")) {
                attr = 2;
            } else if (STREQ(attribute, "italic")) {
                attr = 3;
            } else if (STREQ(attribute, "underline")) {
                attr = 4;
            } else if (STREQ(attribute, "blink")) {
                attr = 5;
            } else if (STREQ(attribute, "reverse")) {
                attr = 7;
            } else if (STREQ(attribute, "invisible")) {
                attr = 8;
            } else if (STREQ(attribute, "strike")) {
                attr = 9;
            } else {
                printf("Unrecognized attribute '%s'\n", attribute);
                goto error;
            }
            printf("\x1b[2%dm", attr);
        } else if (STREQ(command, "tfg")) {
            char *red = shift(argv);
            char *green = shift(argv);
            char *blue = shift(argv);

            if (red == 0 || green == 0 || blue == 0) {
                printf("Please provide tfg with 3 RGB values.\n");
                goto error;
            }
            if (!is_number(red)) {
                printf("Invalid red value '%s'\n", red);
                goto error;
            } else if (!is_number(green)) {
                printf("Invalid green value '%s'\n", green);
                goto error;
            } else if (!is_number(blue)) {
                printf("Invalid blue value '%s'\n", blue);
                goto error;
            }

            printf("\x1b[38;2;%s;%s;%sm", red, green, blue);
        } else if (STREQ(command, "tbg")) {
            char *red = shift(argv);
            char *green = shift(argv);
            char *blue = shift(argv);

            if (red == 0 || green == 0 || blue == 0) {
                printf("Please provide tbg with 3 RGB values.\n");
                goto error;
            }
            if (!is_number(red)) {
                printf("Invalid red value '%s'\n", red);
                goto error;
            } else if (!is_number(green)) {
                printf("Invalid green value '%s'\n", green);
                goto error;
            } else if (!is_number(blue)) {
                printf("Invalid blue value '%s'\n", blue);
                goto error;
            }

            printf("\x1b[48;2;%s;%s;%sm", red, green, blue);
        } else if (STREQ(command, "echo")) {
            char *msg = shift(argv);
            // This is not an error
            if (msg == 0) {
                printf("\n");
                continue;
            }
            printf("%s\n", msg);
        } else {
            int layer;
            int color;
            if (STREQ(command, "fg")) {
                layer = 30;
            } else if (STREQ(command, "bg")) {
                layer = 40;
            } else {
                printf("Unrecognized command '%s'.\n", command);
                goto error;
            }

            char *color_arg = shift(argv);
            if (color_arg == 0) {
                printf("Must provide a color for %s.\n", command);
                goto error;
            }
            if (STREQ(color_arg, "bright")) {
                layer += 60;
                color_arg = shift(argv);
            }
            if (STREQ(color_arg, "black")) {
                color = 0;
            } else if (STREQ(color_arg, "red")) {
                color = 1;
            } else if (STREQ(color_arg, "green")) {
                color = 2;
            } else if (STREQ(color_arg, "yellow")) {
                color = 3;
            } else if (STREQ(color_arg, "blue")) {
                color = 4;
            } else if (STREQ(color_arg, "magenta")) {
                color = 5;
            } else if (STREQ(color_arg, "cyan")) {
                color = 6;
            } else if (STREQ(color_arg, "white")) {
                color = 7;
            } else if (STREQ(color_arg, "default")) {
                color = 9;
            } else {
                printf("Unrecognized color '%s'.\n", color_arg);
                goto error;
            }

            printf("\x1b[%dm", layer + color);
        }
    }

    deinit();

    return 0;

error:
    deinit();
    return 1;
}
