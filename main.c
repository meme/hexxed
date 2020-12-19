#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "buffer.h"
#include "dialogs.h"
#include "panes.h"
#include "render.h"

static const options_t EMPTY_OPT = {
    [0 ... 9] = "      "
};

static void
render_status(const char *path, int width, int height)
{
    attrset(COLOR_PAIR(COLOR_STATUS));
    char status_bar[width + 1];

    memset(status_bar, ' ', width);
    status_bar[width] = '\0';

    char status_message[81];
    snprintf(status_message, sizeof(status_message), "    %-36s%40s",
        path, "    UNK+.00000000`00000000");
    memcpy(status_bar, status_message, sizeof(status_message) - 1 /* NUL */);

    mvaddstr(0, 0, status_bar);
}

static void
render_options(const options_t *options, int width, int height)
{
    attrset(COLOR_PAIR(COLOR_STATUS));

    char options_bar[width + 1];
    memset(options_bar, ' ', width);

    for (int i = 0; i < 10; i++) {
        int start = i * 8;

        char bind[3];
        snprintf(bind, sizeof(bind), "%2d", i + 1);

        attrset(COLOR_PAIR(COLOR_OPTION_KEY));
        mvaddstr(height - 1, start, bind);
        attrset(COLOR_PAIR(COLOR_OPTION_NAME));
        if (i == 9) {
            // Overwrite the F10 input as Quit across all panes.
            //
            mvaddstr(height - 1, start + 2, "Quit  ");
        } else {
            mvaddstr(height - 1, start + 2, (*options)[i]);
        }
    }

    // Draw the hanging characters (if the options window does not fill the
    // screen.)
    // There are 8 characters per option, 6 for the hint and 2 spaces for the
    // bind; 10 options total.
    //
    for (int i = 8 * 10; i < width; i++) {
        mvaddwstr(height - 1, i, FILL_CHAR);
    }
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
driver(int input, int width, int height, pane_t **pane, dialog_t **dialog, buffer_t *buffer)
{
    // If there is a dialog open, capture all input into the dialog.
    //
    if (*dialog != NULL) {
        // If ESC is pressed, close the open dialog.
        //
        if (input == '\x1b') {
            nodelay(stdscr, TRUE);
            int n;
            if ((n = getch()) == ERR) {
                dialog_unpost(*dialog);
                *dialog = NULL;

                // Clear the screen to remove previous render.
                //
                clear();
                render_status(buffer->path, width, height);

                // Render the options of the active pane.
                //
                if (*pane != NULL) {
                    render_options((*pane)->options, width, height);
                }
            } else {
                ungetch(n);
            }
            nodelay(stdscr, FALSE);
        } else {
            dialog_drive(*dialog, input);
            return;
        }
    }

    // Handle pane-independent input, fallthrough to the driver of the active
    // pane.
    //
    switch (input) {
    case '=':
        // Create a calculator dialog in the centre of the screen.
        //
        render_options(&CALCULATOR_OPT, width, height);
        *dialog = calculator_post((height / 2) - (7 / 2), (width / 2) - (72 / 2));
        break;
    case ';':
        render_options(&EMPTY_OPT, width, height);
        *dialog = comments_post(buffer, (height / 2) - (3 / 2), (width / 2) - (72 / 2));
        break;
    case KEY_F(5):
        render_options(&GOTO_OPT, width, height);
        // The pane MUST be active.
        //
        *dialog = goto_post(*pane, (height / 2) - (3 / 2), (width / 2) - (72 / 2));
        break;
    case '\x0a':
    case KEY_ENTER: {
        pane_type_t previous = (*pane)->type;
        pane_unpost(*pane);
        clear();
        render_status(buffer->path, width, height);
        *pane = next_pane(previous, buffer, width, height);
        render_options((*pane)->options, width, height);
    } break;
    default:
        pane_drive(*pane, input);
    }
}

inline static
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
        error("screen must be >86x24");
    }

    // Render the first time to the screen.
    //
    render_status(buffer.path, width, height);
    pane_t *hex_pane = hex_post(&buffer, width, height);
    render_options(hex_pane->options, width, height);

    int input;
    pane_t *active_pane = hex_pane;
    dialog_t *active_popup = NULL;
    while ((input = getch()) != KEY_F(10)) {
        driver(input, width, height, &active_pane, &active_popup, &buffer);
    }

    // If there is a dialog open when the program is closed, free it.
    //
    if (active_popup != NULL) {
        dialog_unpost(active_popup);
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
