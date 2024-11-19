/*  
** Terminal attributes for raw mode taken from 
** <https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html>
*/

#ifndef VIRTERM_H
#define VIRTERM_H

#include <stdint.h>

#define BLACK          0
#define RED            1
#define GREEN          2
#define YELLOW         3
#define BLUE           4
#define MAGENTA        5
#define CYAN           6
#define WHITE          7
#define BRIGHT_BLACK   8
#define BRIGHT_RED     9
#define BRIGHT_GREEN   10
#define BRIGHT_YELLOW  11
#define BRIGHT_BLUE    12
#define BRIGHT_MAGENTA 13
#define BRIGHT_CYAN    14
#define BRIGHT_WHITE   15

#define VT_TAB       0x9
#define VT_RETURN    0xd
#define VT_ESCAPE    0x1b
#define VT_BACKSPACE 0x7f

#define VT_F1     0x80
#define VT_F2     0x81
#define VT_F3     0x82
#define VT_F4     0x83
#define VT_F5     0x84
#define VT_F6     0x85
#define VT_F7     0x86
#define VT_F8     0x87
#define VT_F9     0x88
#define VT_F10    0x89
#define VT_F11    0x8a
#define VT_F12    0x8b
#define VT_INSERT 0x8c
#define VT_DELETE 0x8d
#define VT_HOME   0x8e
#define VT_END    0x8f
#define VT_PGUP   0x90
#define VT_PGDOWN 0x91
#define VT_UP     0x92
#define VT_DOWN   0x93
#define VT_LEFT   0x94
#define VT_RIGHT  0x95

int vt_init(void);
void vt_deinit(void);

void vt_cook(void);
void vt_rare(void);
void vt_raw(void);

void vt_keypad(int enable);
void vt_echo(int enable);
int vt_delay(int delay);

char vt_key(void);
void vt_emit(char c);

void vt_move(int row, int col);

void vt_set_color(int fg, int bg);
void vt_set_tru_fg(uint8_t r, uint8_t g, uint8_t b);
void vt_set_tru_bg(uint8_t r, uint8_t g, uint8_t b);

void vt_reset_mode(void);
void vt_clear(void);

#endif
