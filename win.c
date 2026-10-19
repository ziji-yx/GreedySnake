#define _WIN32_WINNT 0x0600

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include "win.h"

static HANDLE output_handle = INVALID_HANDLE_VALUE;
static WORD default_attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
static CONSOLE_CURSOR_INFO default_cursor_info;
static int cursor_info_saved = 0;

void console_init(void)
{
    output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTitleW(L"Greedy Snake | 贪吃蛇");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (output_handle != INVALID_HANDLE_VALUE)
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(output_handle, &info))
        {
            default_attributes = info.wAttributes;
        }
        if (GetConsoleCursorInfo(output_handle, &default_cursor_info))
        {
            cursor_info_saved = 1;
        }
    }
}

void console_restore(void)
{
    console_show_cursor();
    console_set_color(COLOR_DEFAULT);
}

void console_clear(void)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD written;
    COORD home = {0, 0};
    DWORD length;

    if (output_handle == INVALID_HANDLE_VALUE)
    {
        system("cls");
        return;
    }

    if (!GetConsoleScreenBufferInfo(output_handle, &info))
    {
        return;
    }

    length = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;
    FillConsoleOutputCharacterA(output_handle, ' ', length, home, &written);
    FillConsoleOutputAttribute(output_handle, default_attributes, length, home, &written);
    SetConsoleCursorPosition(output_handle, home);
}

void console_gotoxy(int x, int y)
{
    COORD position;
    position.X = (SHORT)x;
    position.Y = (SHORT)y;

    if (output_handle != INVALID_HANDLE_VALUE)
    {
        SetConsoleCursorPosition(output_handle, position);
    }
}

void console_hide_cursor(void)
{
    CONSOLE_CURSOR_INFO info;

    if (output_handle == INVALID_HANDLE_VALUE || !GetConsoleCursorInfo(output_handle, &info))
    {
        return;
    }

    info.bVisible = FALSE;
    SetConsoleCursorInfo(output_handle, &info);
}

void console_show_cursor(void)
{
    CONSOLE_CURSOR_INFO info;

    if (output_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (cursor_info_saved)
    {
        SetConsoleCursorInfo(output_handle, &default_cursor_info);
        return;
    }

    if (GetConsoleCursorInfo(output_handle, &info))
    {
        info.bVisible = TRUE;
        SetConsoleCursorInfo(output_handle, &info);
    }
}

void console_flush_input(void)
{
    while (_kbhit())
    {
        (void)_getch();
    }
}

void console_set_color(ConsoleColor color)
{
    WORD attributes;

    if (output_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    switch (color)
    {
    case COLOR_DIM:
        attributes = FOREGROUND_INTENSITY;
        break;
    case COLOR_GREEN:
        attributes = FOREGROUND_GREEN;
        break;
    case COLOR_BRIGHT_GREEN:
        attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case COLOR_RED:
        attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
        break;
    case COLOR_MAGENTA:
        attributes = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
    case COLOR_YELLOW:
        attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        break;
    case COLOR_CYAN:
        attributes = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        break;
    case COLOR_DEFAULT:
    default:
        attributes = default_attributes;
        break;
    }

    SetConsoleTextAttribute(output_handle, attributes);
}
