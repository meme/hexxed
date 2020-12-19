#pragma once

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#include <form.h>
#include <ncurses.h>
#include <wchar.h>

// Thanks, ncurses.
//
#ifdef scroll
#undef scroll
#endif

// Options to be rendered while the dialog is active. The position indicates
// the corresponding input: KEY_F(N).
//
typedef char options_t[10][7];

enum {
    COLOR_STATUS = 1,
    COLOR_SELECTED,
    COLOR_STANDARD,
    COLOR_OPTION_NAME,
    COLOR_OPTION_KEY,
    HIGHLIGHT_BLUE,
    HIGHLIGHT_WHITE,
    COLOR_BRIGHT_WHITE
};

static const wchar_t *FILL_CHAR = L"▒";

