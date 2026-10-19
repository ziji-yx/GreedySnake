#ifndef WIN_H
#define WIN_H

typedef enum
{
    COLOR_DEFAULT,
    COLOR_DIM,
    COLOR_GREEN,
    COLOR_BRIGHT_GREEN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_CYAN
} ConsoleColor;

void console_init(void);
void console_restore(void);
void console_clear(void);
void console_gotoxy(int x, int y);
void console_hide_cursor(void);
void console_show_cursor(void);
void console_flush_input(void);
void console_set_color(ConsoleColor color);

#endif
