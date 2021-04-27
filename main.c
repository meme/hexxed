#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "buffer.h"
#include "panes.h"
#include "render.h"

int calculator_eval(buffer_t *buffer, const char *input, int64_t *result);

static char*
trim(char *str)
{
	char *end = NULL;

    // Remove leading spaces.
    //
	while (*str == ' ') {
		str++;
    }

    // If the string is ALL spaces.
    //
	if (*str == 0) {
		return str;
    }

	end = str + strnlen(str, 128) - 1;

	while(end > str && *end == ' ') {
		end--;
    }

	*(end + 1) = '\0';
	return str;
}

static pane_t*
next_pane(pane_type_t type, buffer_t *buffer, int width, int height)
{
    if (type == PANE_HEX) {
        return text_post(buffer, width, height);
    } else if (type == PANE_TEXT) {
        return hex_post(buffer, width, height);
    } else {
        __builtin_unreachable();
    }
}

static void
driver(int input, int width, int height, pane_t **pane, buffer_t *buffer)
{
    switch (input) {
    case '=':
        render_options(&EMPTY_OPT);
        prompt_calculator(buffer);
        goto reset;
    case ';': {
        render_options(&EMPTY_OPT);

        char name[72];
        snprintf(name, sizeof(name), "Comment at offset .%08lx`%08lx",
            buffer->cursor & 0xffffffff00000000, buffer->cursor & 0x00000000ffffffff);

        char *user_input = NULL;
        {
            const char *comment = buffer_lookup_comment(buffer, buffer->cursor);
            prompt_input(name, comment, &user_input);
        }

        if (user_input != NULL) {
            char *comment = trim(user_input);

            if (strlen(comment) == 0) {
                buffer_remove_comment(buffer, buffer->cursor);
            } else {
                buffer_add_comment(buffer, buffer->cursor, comment);
            }

            free(user_input);
        }
        goto reset;
    }
    case KEY_F(5): {
        render_options(&EMPTY_OPT);

        char *user_input = NULL;
        prompt_input("Goto", NULL, &user_input);

        int64_t result = -1;
        if (user_input != NULL && calculator_eval(buffer, user_input, &result) == 0 && result >= 0) {
            pane_scroll(*pane, (uint64_t) result);
        }

        free(user_input);
        goto reset;
    }
    case KEY_F(9): {
        size_t comments_size = g_hash_table_size(buffer->comments);
        if (comments_size == 0) {
            prompt_error("No names.");
            goto reset;
        }

        char **comments_data = malloc(sizeof(char*) * comments_size);

        GHashTableIter i;
        gpointer key, value;
        g_hash_table_iter_init(&i, buffer->comments);
        int n = 0;
        while (g_hash_table_iter_next(&i, &key, &value)) {
            char *comment;
            asprintf(&comment, "%08x  %s", (uint32_t) (GPOINTER_TO_SIZE(key) & 0x00000000ffffffff), (char*) value);
            comments_data[n++] = comment;
        }

        int selected = prompt_menu("Names", (const char**) comments_data, comments_size, 64, 0);

        // Find the address of the selected name.
        //
        g_hash_table_iter_init(&i, buffer->comments);
        n = 0;
        while (g_hash_table_iter_next(&i, &key, &value)) {
            if (n++ == selected) {
                pane_scroll(*pane, GPOINTER_TO_SIZE(key));
            }
        }

        for (int j = 0; j < comments_size; j++) {
            free(comments_data[j]);
        }

        free(comments_data);
        goto reset;
    }
    case '\x0a':
    case KEY_ENTER: {
        pane_type_t previous = (*pane)->type;
        pane_unpost(*pane);
        clear();
        render_status(buffer->path);
        *pane = next_pane(previous, buffer, width, height);
        goto reset;
    }
    case KEY_F(3):
        if (!buffer->editable) {
            if (buffer_try_reopen(buffer)) {
                prompt_error("The file could not be opened as writable.");
                break;
            }
        }
        goto drive;
    default:
drive:
        render_status(buffer->path);
        render_options((*pane)->options);
        pane_drive(*pane, input);
    }

    return;
reset:
    clear();
    goto drive;
}

inline static void __attribute__ ((noreturn))
error(const char *message)
{
    endwin();
    fprintf(stderr, "error: %s\n", message);
    exit(1);
}

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "error: no path provided\n");
        return 1;
    }

    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(15);

    // Ensure colours are active.
    //
    if (!has_colors() || start_color() != OK) {
        error("screen does not support colors");
    }

    if (can_change_color() && COLORS >= 16) {
        init_color(COLOR_BRIGHT_WHITE, 1000, 1000, 1000);
    }

    if (COLORS >= 16) {
        init_pair(HIGHLIGHT_WHITE, COLOR_BLACK, COLOR_BRIGHT_WHITE);
    } else {
        init_pair(HIGHLIGHT_WHITE, COLOR_BLACK, COLOR_WHITE);
    }

    init_pair(COLOR_STATUS, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_SELECTED, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_OPTION_NAME, COLOR_BLACK, COLOR_WHITE);
    init_pair(COLOR_OPTION_KEY, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_STANDARD, COLOR_WHITE, COLOR_BLACK);
    init_pair(HIGHLIGHT_BLUE, COLOR_WHITE, COLOR_BLUE);

    // Exit if the terminal is too small.
    //
    int height, width;
    getmaxyx(stdscr, height, width);

    if (width < 86 || height < 24) {
        error("screen must be >=86x24");
    }

    // Make the default cursor invisible.
    //
    curs_set(0);
    clear();

    buffer_t buffer;
    if (buffer_open(&buffer, argv[1]) != 0) {
        error("cannot open path");
    }

    // Render the first time to the screen.
    //
    render_status(buffer.path);
    pane_t *hex_pane = hex_post(&buffer, width, height);
    render_options(hex_pane->options);

    int input;
    pane_t *active_pane = hex_pane;
    while ((input = getch()) != KEY_F(10)) {
        driver(input, width, height, &active_pane, &buffer);
    }

    if (active_pane != NULL) {
        pane_unpost(active_pane);
    }

    if (buffer_close(&buffer) != 0) {
        error("cannot close buffer");
    }

    endwin();
    return 0;
}
