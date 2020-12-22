#include "render.h"

#include <assert.h>
#include <string.h>

int calculator_eval(const char *input, int64_t *result);

static inline int
input_is_esc(int input)
{
    if (input == '\x1b') {
        nodelay(stdscr, TRUE);
        int n;
        if ((n = getch()) == ERR) {
            return 1;
        } else {
            ungetch(n);
        }
        nodelay(stdscr, FALSE);
    }

    return 0;
}

void
render_status(const char *path)
{
    int height, width;
    getmaxyx(stdscr, height, width);

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

void
render_options(const options_t *options)
{
    int height, width;
    getmaxyx(stdscr, height, width);

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

void
render_border(WINDOW *window)
{
    static const cchar_t BOX_VERT = { 0, { L'║' } };
    static const cchar_t BOX_HORIZ = { 0, { L'═' } };
    static const cchar_t BOX_TOP_LEFT = { 0, { L'╔' } };
    static const cchar_t BOX_TOP_RIGHT = { 0, { L'╗' } };
    static const cchar_t BOX_BOTTOM_LEFT = { 0, { L'╚' } };
    static const cchar_t BOX_BOTTOM_RIGHT = { 0, { L'╝' } };

    wborder_set(window, &BOX_VERT, &BOX_VERT, &BOX_HORIZ, &BOX_HORIZ,
            &BOX_TOP_LEFT, &BOX_TOP_RIGHT, &BOX_BOTTOM_LEFT, &BOX_BOTTOM_RIGHT);
}

static int
calculator_driver(int input, WINDOW *window, FORM *form, FIELD **fields)
{
    if (input_is_esc(input)) {
        return 0;
    }

    switch (input) {
    case KEY_ENTER:
    case '\x0a':
        // Sychronize the field so we can get the buffer data.
        //
        form_driver(form, REQ_VALIDATION);

        char *expression = field_buffer(fields[0], 0);

        // Evaluate the input. If an error occurs, zero out the result fields.
        int64_t result;
        if (calculator_eval(expression, &result) != 0) {
            set_field_buffer(fields[1], 0, "Sig:0");
            set_field_buffer(fields[2], 0, "Uns:0");
            set_field_buffer(fields[3], 0, "Bin:0000000000000000000000000000000000000000000000000000000000000000");
            set_field_buffer(fields[4], 0, "Hex:00000000`00000000");
        } else {
            char sig_message[69];
            snprintf(sig_message, sizeof(sig_message), "Sig:%lld", result);
            set_field_buffer(fields[1], 0, sig_message);

            char uns_message[69];
            snprintf(uns_message, sizeof(uns_message), "Uns:%llu", result);
            set_field_buffer(fields[2], 0, uns_message);

            char bin_message[69];
            snprintf(bin_message, sizeof(bin_message), "Bin:");
            for (int i = 0; i < 64; i++) {
                snprintf(bin_message + i + 4, sizeof(bin_message) - i - 4, "%d", (result & ((uint64_t) 1 << (63 - i))) != 0);
            }
            set_field_buffer(fields[3], 0, bin_message);

            char hex_message[69];
            snprintf(hex_message, sizeof(hex_message), "Hex:%08x`%08x",
                    (uint32_t) ((result & 0xffffffff00000000) >> 32),
                    (uint32_t) (result & 0x00000000ffffffff));
            set_field_buffer(fields[4], 0, hex_message);
        }

        refresh();
        pos_form_cursor(form);
        break;
    case KEY_LEFT:
        form_driver(form, REQ_PREV_CHAR);
        break;
    case KEY_RIGHT:
        form_driver(form, REQ_NEXT_CHAR);
        break;
    case KEY_BACKSPACE:
    case '\x7f':
        form_driver(form, REQ_DEL_PREV);
        break;
    // DEL.
    //
    case KEY_DC:
        form_driver(form, REQ_DEL_CHAR);
        break;
    default:
        form_driver(form, input);
        break;
    }

    wrefresh(window);
    return 1;
}

void
prompt_calculator()
{
    // Create a window to contain the calculator, factoring in the border sizes.
    // +2 for the vertical border, and +4 for the left and right padding on the
    // horizontal border.
    //
    int height, width;
    getmaxyx(stdscr, height, width);
    WINDOW *window = newwin(5 + 2, 68 + 4, (height / 2) - (7 / 2), (width / 2) - (72 / 2));
    assert(window != NULL);

    render_border(window);

    FIELD *fields[6];
    fields[0] = new_field(1, 68, 0, 1, 0, 0);
    fields[1] = new_field(1, 68, 1, 1, 0, 0);
    fields[2] = new_field(1, 68, 2, 1, 0, 0);
    fields[3] = new_field(1, 68, 3, 1, 0, 0);
    fields[4] = new_field(1, 68, 4, 1, 0, 0);
    fields[5] = NULL;
    assert(fields[0] != NULL && fields[1] != NULL && fields[2] != NULL && fields[3] != NULL && fields[4] != NULL);

    set_field_buffer(fields[1], 0, "Sig:0");
    set_field_buffer(fields[2], 0, "Uns:0");
    set_field_buffer(fields[3], 0, "Bin:0000000000000000000000000000000000000000000000000000000000000000");
    set_field_buffer(fields[4], 0, "Hex:00000000`00000000");

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[3], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts(fields[4], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);

    // Underline the field to indicate it can be edited.
    //
    set_field_back(fields[0], A_UNDERLINE);

    FORM *form = new_form(fields);
    assert(form != NULL);

    set_form_win(form, window);
    set_form_sub(form, derwin(window, 9 - 4, 72 - 2, 1, 1));
    post_form(form);

    // Draw the header for the dialog, and reset the cursor back to the input.
    //
    mvwprintw(window, 0, 72 / 2 - ((sizeof(" Calculator ") - 1) / 2), " Calculator ");
    wmove(window, 1, 2);

    refresh();
    wrefresh(window);

    // Set the cursor to visible.
    //
    curs_set(1);

    while (calculator_driver(getch(), window, form, fields));

    // Restore the cursor state.
    //
    curs_set(0);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);
    free_field(fields[4]);
    delwin(window);
}

static int
input_driver(int input, WINDOW *window, FORM *form, FIELD **fields)
{
    switch (input) {
    case KEY_ENTER:
    case '\x0a':
        // Sychronize the field so we can get the buffer data.
        //
        form_driver(form, REQ_VALIDATION);
        refresh();
        pos_form_cursor(form);
        return 0;
    case KEY_LEFT:
        form_driver(form, REQ_PREV_CHAR);
        break;
    case KEY_RIGHT:
        form_driver(form, REQ_NEXT_CHAR);
        break;
    case KEY_BACKSPACE:
    case '\x7f':
        form_driver(form, REQ_DEL_PREV);
        break;
    // DEL.
    //
    case KEY_DC:
        form_driver(form, REQ_DEL_CHAR);
        break;
    default:
        form_driver(form, input);
        break;
    }

    wrefresh(window);
    return 1;
}

size_t
prompt_input(const char *title, const char *placeholder, char **user_input)
{
    // Create a window to contain the menu, factoring in the border sizes.
    // +2 for the vertical border, and +4 for the left and right padding on the
    // horizontal border.
    //
    int height, width;
    getmaxyx(stdscr, height, width);
    WINDOW *window = newwin(1 + 2, 68 + 4,  (height / 2) - (3 / 2), (width / 2) - (72 / 2));
    assert(window != NULL);

    render_border(window);

    FIELD *fields[2];
    fields[0] = new_field(1, 68, 0, 1, 0, 0);
    fields[1] = NULL;
    assert(fields[0] != NULL);

    set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

    // Underline the field to indicate it can be edited.
    //
    set_field_back(fields[0], A_UNDERLINE);

    FORM *form = new_form(fields);
    assert(form != NULL);

    set_form_win(form, window);
    set_form_sub(form, derwin(window, 5 - 4, 72 - 2, 1, 1));
    post_form(form);

    // Draw the header for the dialog, and reset the cursor back to the input.
    //
    int start = 72 / 2 - (strlen(title) / 2);
    mvwprintw(window, 0, start - 1, " ");
    mvwprintw(window, 0, start, title);
    mvwprintw(window, 0, start + strlen(title), " ");
    wmove(window, 1, 2);

    if (placeholder != NULL) {
        set_field_buffer(fields[0], 0, placeholder);
        while (*placeholder++ != '\0') {
            form_driver(form, REQ_NEXT_CHAR);
        }
    }

    refresh();
    wrefresh(window);

    // Set the cursor to visible.
    //
    curs_set(1);

    int input;
    while ((input = getch()) && !input_is_esc(input)) {
        if (input_driver(input, window, form, fields) == 0) {
            break;
        }
    }

    // If ESC is pressed "cancel" the input request and set user_input to NULL.
    //
    size_t input_size;
    if (input_is_esc(input)) {
        *user_input = NULL;
        input_size = 0;
    } else {
       char *input = field_buffer(fields[0], 0);
        *user_input = strdup(input);
        input_size = strlen(*user_input);
    }

    // Restore the cursor state.
    //
    curs_set(0);

    unpost_form(form);
    free_form(form);
    free_field(fields[0]);
    delwin(window);

    return input_size;
}