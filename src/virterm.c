#include "virterm.h" // Needs to be first line for unity build reasons
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <wchar.h>
#include <windows.h>
#include <conio.h>

static DWORD vt_org_out_mode = 0;
static DWORD vt_org_in_mode = 0;
static HANDLE vt_out = INVALID_HANDLE_VALUE;
static HANDLE vt_in = INVALID_HANDLE_VALUE;
static int vt_keypad_enable = 0;
static int vt_key_delay = -1;

char vt_key(void)
{
    char buffer;
    DWORD read;
    if (vt_key_delay >= 0) {
        INPUT_RECORD records[100];
        DWORD read;
        int flag = 0;
            // DWORD n;
            // GetNumberOfConsoleInputEvents(vt_in, &n);
            // printf("%ld\n", n);
        PeekConsoleInput(vt_in, records, 100, &read);
        for (DWORD i = 0; i < read; ++i) {
            if (records[i].EventType == KEY_EVENT && records[i].Event.KeyEvent.bKeyDown) {
                flag = 1;
            }
        }
        if (!flag) {
            FlushConsoleInputBuffer(vt_in);
            DWORD result = WaitForSingleObject(vt_in, vt_key_delay * 100);
            if (result == WAIT_TIMEOUT) {
                return 0;
            }
        }
    }
    ReadConsole(vt_in, &buffer, 1, &read, NULL);
    if (vt_keypad_enable && buffer == VT_ESCAPE) {
        char code[4];
        ReadConsole(vt_in, code, 4, &read, NULL);
        if (code[0] == 0x4f) {
            buffer = code[1] + 0x30;
        } else if (read == 4) {
            if (code[1] == '1') {
                if (code[2] == '5') {
                    buffer = VT_F5;
                } else if (code[2] == '7') {
                    buffer = VT_F6;
                } else if (code[2] == '8') {
                    buffer = VT_F7;
                } else if (code[2] == '9') {
                    buffer = VT_F8;
                }
            } else if (code[1] == '2') {
                if (code[2] == '0') {
                    buffer = VT_F9;
                } else if (code[2] == '1') {
                    buffer = VT_F10;
                } else if (code[2] == '3') {
                    buffer = VT_F11;
                } else if (code[2] == '4') {
                    buffer = VT_F12;
                }
            }
        } else if (read == 3) {
            if (code[1] == '2') {
                buffer = VT_INSERT;
            } else if (code[1] == '3') {
                buffer = VT_DELETE;
            } else if (code[1] == '5') {
                buffer = VT_PGUP;
            } else if (code[1] == '6') {
                buffer = VT_PGDOWN;
            }
        } else if (read == 2) {
            if (code[1] == 'A') {
                buffer = VT_UP;
            } else if (code[1] == 'B') {
                buffer = VT_DOWN;
            } else if (code[1] == 'C') {
                buffer = VT_RIGHT;
            } else if (code[1] == 'D') {
                buffer = VT_LEFT;
            } else if (code[1] == 'H') {
                buffer = VT_HOME;
            } else if (code[1] == 'F') {
                buffer = VT_END;
            }
        }
        FlushConsoleInputBuffer(vt_in);
    }

    return buffer;
}

void vt_emit(char c)
{
    WriteConsole(vt_out, &c, 1, NULL, NULL);
}

static BOOL WINAPI vt_handler(DWORD ctrl_type)
{
    if (ctrl_type == CTRL_C_EVENT) {
        vt_deinit();
        ExitProcess(ERROR_CONTROL_C_EXIT);
    }
    return FALSE;
}

int vt_init(void)
{
    // Set output mode to handle virtual terminal sequences
    vt_out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (vt_out == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    vt_in = GetStdHandle(STD_INPUT_HANDLE);
    if (vt_in == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    if (!GetConsoleMode(vt_out, &vt_org_out_mode))
    {
        return 0;
    }
    if (!GetConsoleMode(vt_in, &vt_org_in_mode))
    {
        return 0;
    }

    DWORD new_out_mode = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD new_in_mode = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD out_mode = vt_org_out_mode | new_out_mode;
    if (!SetConsoleMode(vt_out, out_mode))
    {
        // we failed to set both modes, try to step down mode gracefully.
        new_out_mode = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        out_mode = vt_org_out_mode | new_out_mode;
        if (!SetConsoleMode(vt_out, out_mode))
        {
            // Failed to set any VT mode, can't do anything here.
            return 0;
        }
    }

    DWORD in_mode = vt_org_in_mode | new_in_mode;
    if (!SetConsoleMode(vt_in, in_mode))
    {
        // Failed to set VT input mode, can't do anything here.
        return 0;
    }

    // Set cbreak handler
    SetConsoleCtrlHandler(vt_handler, TRUE);

    // Flush input buffer
    FlushConsoleInputBuffer(vt_in);

    // Enable the alternate buffer
    printf("\x1b[?1049h");

    return 1;
}

void vt_deinit(void)
{
    // Disable the alternate buffer
    printf("\x1b[?1049l");

    // Check for valid handles
    if (vt_out == INVALID_HANDLE_VALUE || vt_in == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SetConsoleMode(vt_out, vt_org_out_mode);
    SetConsoleMode(vt_in, vt_org_in_mode);
}

void vt_cook(void)
{
    if (vt_out == INVALID_HANDLE_VALUE || vt_in == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD out_mode, in_mode;
    if (!GetConsoleMode(vt_out, &out_mode))
    {
        return;
    }
    if (!GetConsoleMode(vt_in, &in_mode))
    {
        return;
    }

    out_mode |= ENABLE_PROCESSED_OUTPUT;
    in_mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT |
        ENABLE_PROCESSED_INPUT;

    SetConsoleMode(vt_out, out_mode);
    SetConsoleMode(vt_in, in_mode);
}

void vt_rare(void)
{
    if (vt_out == INVALID_HANDLE_VALUE || vt_in == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD out_mode, in_mode;
    if (!GetConsoleMode(vt_out, &out_mode))
    {
        return;
    }
    if (!GetConsoleMode(vt_in, &in_mode))
    {
        return;
    }

    out_mode &= ~(ENABLE_WRAP_AT_EOL_OUTPUT);
    in_mode |= ENABLE_PROCESSED_INPUT;
    in_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

    SetConsoleMode(vt_out, out_mode);
    SetConsoleMode(vt_in, in_mode);
}

void vt_raw(void)
{
    if (vt_out == INVALID_HANDLE_VALUE || vt_in == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD out_mode, in_mode;
    if (!GetConsoleMode(vt_out, &out_mode))
    {
        return;
    }
    if (!GetConsoleMode(vt_in, &in_mode))
    {
        return;
    }

    out_mode &= ~(ENABLE_WRAP_AT_EOL_OUTPUT);
    in_mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT |
            ENABLE_PROCESSED_INPUT);

    SetConsoleMode(vt_out, out_mode);
    SetConsoleMode(vt_in, in_mode);
}

void vt_keypad(int enable)
{
    vt_keypad_enable = enable;
}

void vt_echo(int enable)
{
    if (vt_in == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD in_mode;
    if (!GetConsoleMode(vt_in, &in_mode))
    {
        return;
    }

    if (enable) {
        in_mode |= (ENABLE_ECHO_INPUT);
    } else {
        in_mode &= ~(ENABLE_ECHO_INPUT);
    }

    SetConsoleMode(vt_in, in_mode);
}

int vt_delay(int delay)
{
    int old_delay = vt_key_delay;
    vt_key_delay = delay;
    return old_delay;
}
#else
#include <unistd.h>
#include <termios.h>
#include <signal.h>

static struct termios vt_term_mode;

char vt_key(void)
{
    return getchar();
}

void vt_emit(char c)
{
    putchar(c);
}

static void vt_handler(int signal)
{
    if (signal == SIGINT) {
        vt_deinit();
        _exit(128);
    }
}

int vt_init(void)
{
    // Preserve terminal attributes
    tcgetattr(STDIN_FILENO, &vt_term_mode);

    // Register custom signal handler for cbreak
    signal(SIGINT, vt_handler);

    // Enable the alternate buffer
    printf("\x1b[?1049h");

    return 1;
}

void vt_deinit(void)
{
    // Disable the alternate buffer
    printf("\x1b[?1049l");

    // Restore terminal attributes
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &vt_term_mode);
}

void vt_cook(void)
{
    struct termios term_attr;
    tcgetattr(STDIN_FILENO, &term_attr);

    term_attr.c_iflag |= IXON | ICRNL | BRKINT | INPCK | ISTRIP;
    term_attr.c_oflag |= OPOST;
    term_attr.c_lflag |= ECHO | ICANON | ISIG | IEXTEN;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);
}

void vt_rare(void)
{
    struct termios term_attr;
    tcgetattr(STDIN_FILENO, &term_attr);

    term_attr.c_iflag &= ~(ICRNL | INPCK | ISTRIP);
    term_attr.c_oflag &= ~(0);
    term_attr.c_cflag |= CS8;
    term_attr.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);
}

void vt_raw(void)
{
    struct termios term_attr;
    tcgetattr(STDIN_FILENO, &term_attr);

    term_attr.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    term_attr.c_oflag &= ~(0);
    term_attr.c_cflag |= CS8;
    term_attr.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);
}

void vt_keypad(int enable)
{
    
}

void vt_echo(int enable)
{
    struct termios term_attr;
    tcgetattr(STDIN_FILENO, &term_attr);

    if (enable) {
        term_attr.c_lflag |= (ECHO);
    } else {
        term_attr.c_lflag &= ~(ECHO);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);
}

int vt_delay(int delay)
{
    struct termios term_attr;
    tcgetattr(STDIN_FILENO, &term_attr);

    int old_delay;
    if (term_attr.c_cc[VMIN] == 0) {
        old_delay = term_attr.c_cc[VTIME];
    } else {
        old_delay = -1;
    }
    if (delay < 0) {
        term_attr.c_cc[VMIN] = 1;
        term_attr.c_cc[VTIME] = 0;
    } else {
        term_attr.c_cc[VMIN] = 0;
        term_attr.c_cc[VTIME] = delay;
    }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &term_attr);

    return old_delay;
}
#endif

static int vt_row = 0;
static int vt_column = 0;

void vt_move(int row, int col)
{
    vt_row = row;
    vt_column = col;

    printf("\x1b[%d;%dH", vt_row, vt_column);
}

void vt_set_color(int fg, int bg)
{
    printf("\x1b[38;5;%dm", fg);
    printf("\x1b[48;5;%dm", bg);
}

void vt_set_tru_fg(uint8_t r, uint8_t g, uint8_t b)
{
    printf("\x1b[38;2;%hhu;%hhu;%hhum", r, g, b);
}

void vt_set_tru_bg(uint8_t r, uint8_t g, uint8_t b)
{
    printf("\x1b[48;2;%hhu;%hhu;%hhum", r, g, b);
}

void vt_reset_mode(void)
{
    printf("\x1b[0m");
}

void vt_clear(void)
{
    printf("\x1b[2J");
}
