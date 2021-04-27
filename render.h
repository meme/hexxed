#pragma once

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <wchar.h>

#include "buffer.h"

// Thanks, ncurses.
//
#ifdef scroll
#undef scroll
#endif

// Options to be rendered while the dialog is active. The position indicates
// the corresponding input: KEY_F(N).
//
typedef char options_t[10][7];

static const options_t EMPTY_OPT = {
    [0 ... 9] = "      "
};

static const wchar_t *FILL_CHAR = L"▒";

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

void render_status(const char *path);
void render_options(const options_t *options);
void render_border(WINDOW *window);

// Prompts the user for a message setting user_input which must be free'd.
// user_input is set to NULL if the prompt is cancelled with ESC.
// The state of the screen is UNDEFINED after this function returns.
//
size_t prompt_input(const char *title, const char *placeholder, char **user_input);
// Prompts the user for a menu item returning the index into options.
// The result is -1 if the prompt is cancelled with ESC.
// The state of the screen is UNDEFINED after this function returns.
int prompt_menu(const char *title, const char **options, size_t options_size, int width, int start_item);
// Displays a non-fatal error to the user.
//
void prompt_error(const char *message);
// The state of the screen is UNDEFINED after this function returns.
//
void prompt_calculator(buffer_t *buffer);
