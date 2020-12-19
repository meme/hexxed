#include "dialogs.h"
#include "render.h"

#include <stdlib.h>
#include <assert.h>

int calculator_eval(const char *input, int64_t *result);

void
dialog_unpost(dialog_t *dialog)
{
    dialog->unpost(dialog->user_data);
    free(dialog);
}

void
dialog_drive(dialog_t *dialog, int input)
{
    dialog->driver(dialog->user_data, input);
}

// For terminals that support wide characters, render a box around the window.
//
static void
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


const options_t GOTO_OPT = {
    [0 ... 9] = "      "
};

typedef struct {
    WINDOW *window;
    FORM *form;
    FIELD *fields[2];
    pane_t *pane;
} goto_dialog_t;

static void
goto_unpost(void *user_data)
{
    goto_dialog_t *state = (goto_dialog_t*) user_data;

    // Restore the cursor state.
    //
    curs_set(0);

    unpost_form(state->form);
    free_form(state->form);
    free_field(state->fields[0]);
    delwin(state->window);

    free(user_data);
}

static void
goto_driver(void *user_data, int input)
{
    goto_dialog_t *state = (goto_dialog_t*) user_data;
    WINDOW *window = state->window;
    FORM *form = state->form;
    FIELD **fields = state->fields;

    switch (input) {
    case KEY_ENTER:
    case '\x0a':
        // Sychronize the field so we can get the buffer data.
        //
        form_driver(form, REQ_VALIDATION);

        char *expression = field_buffer(fields[0], 0);

        int64_t result = -1;
        if (calculator_eval(expression, &result) == 0 && result >= 0) {
            pane_scroll(state->pane, (uint64_t) result);
            // Send an ESC to the driver to close the dialog, this is... a hack.
            //
            ungetch('\x1b');
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
}

dialog_t*
goto_post(pane_t *pane, int top, int left)
{
    // Create a window to contain the menu, factoring in the border sizes.
    // +2 for the vertical border, and +4 for the left and right padding on the
    // horizontal border.
    //
    WINDOW *window = newwin(1 + 2, 68 + 4, top, left);
    assert(window != NULL);

    render_border(window);

    goto_dialog_t *state = malloc(sizeof(goto_dialog_t));

    FIELD **fields = (FIELD**) state->fields;
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
    mvwprintw(window, 0, 72 / 2 - ((sizeof(" Goto ") - 1) / 2), " Goto ");
    wmove(window, 1, 2);

    refresh();
    wrefresh(window);

    state->window = window;
    state->form = form;
    state->pane = pane;

    dialog_t *dialog = malloc(sizeof(dialog_t));
    dialog->driver = goto_driver;
    dialog->unpost = goto_unpost;
    dialog->user_data = (void*) state;

    // Set the cursor to visible.
    //
    curs_set(1);

    return dialog;
}

const options_t CALCULATOR_OPT = {
    [0 ... 9] = "      "
};

typedef struct {
    WINDOW *window;
    FORM *form;
    FIELD *fields[6];
} calculator_dialog_t;

static void
calculator_unpost(void *user_data)
{
    calculator_dialog_t *state = (calculator_dialog_t*) user_data;
    WINDOW *window = state->window;
    FORM *form = state->form;
    FIELD **fields = state->fields;

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

    free(user_data);
}

static void
calculator_driver(void *user_data, int input)
{
    calculator_dialog_t *state = (calculator_dialog_t*) user_data;
    WINDOW *window = state->window;
    FORM *form = state->form;
    FIELD **fields = state->fields;

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
            snprintf(sig_message, sizeof(sig_message), "Sig:%ld", result);
            set_field_buffer(fields[1], 0, sig_message);

            char uns_message[69];
            snprintf(uns_message, sizeof(uns_message), "Uns:%lu", result);
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
}

dialog_t*
calculator_post(int top, int left)
{
    // Create a window to contain the calculator, factoring in the border sizes.
    // +2 for the vertical border, and +4 for the left and right padding on the
    // horizontal border.
    //
    WINDOW *window = newwin(5 + 2, 68 + 4, top, left);
    assert(window != NULL);

    render_border(window);

    calculator_dialog_t *state = malloc(sizeof(calculator_dialog_t));

    FIELD **fields = (FIELD**) state->fields;
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

    state->window = window;
    state->form = form;

    dialog_t *dialog = malloc(sizeof(dialog_t));
    dialog->driver = calculator_driver;
    dialog->unpost = calculator_unpost;
    dialog->user_data = (void*) state;

    // Set the cursor to visible.
    //
    curs_set(1);

    return dialog;
}

typedef struct {
    buffer_t *buffer;
    WINDOW *window;
    FORM *form;
    FIELD *fields[2];
} comment_dialog_t;

static void
comments_unpost(void *user_data)
{
    comment_dialog_t *state = (comment_dialog_t*) user_data;

    // Restore the cursor state.
    //
    curs_set(0);

    unpost_form(state->form);
    free_form(state->form);
    free_field(state->fields[0]);
    delwin(state->window);

    free(user_data);
}

static void
comments_driver(void *user_data, int input)
{
    comment_dialog_t *state = (comment_dialog_t*) user_data;
    WINDOW *window = state->window;
    FORM *form = state->form;
    FIELD **fields = state->fields;

    switch (input) {
    case KEY_ENTER:
    case '\x0a':
        // Sychronize the field so we can get the buffer data.
        //
        form_driver(form, REQ_VALIDATION);

        char *comment = trim(field_buffer(fields[0], 0));

        if (strlen(comment) == 0) {
            buffer_remove_comment(state->buffer, state->buffer->cursor);
        } else {
            buffer_add_comment(state->buffer, state->buffer->cursor, trim(comment));
        }

        ungetch('\x1b');

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
}

dialog_t*
comments_post(buffer_t *buffer, int top, int left)
{
    // Create a window to contain the menu, factoring in the border sizes.
    // +2 for the vertical border, and +4 for the left and right padding on the
    // horizontal border.
    //
    WINDOW *window = newwin(1 + 2, 68 + 4, top, left);
    assert(window != NULL);

    render_border(window);

    comment_dialog_t *state = malloc(sizeof(comment_dialog_t));

    FIELD **fields = (FIELD**) state->fields;
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
    char name[72];
    snprintf(name, sizeof(name), " Comment at offset .%08lx`%08lx ",
                buffer->cursor & 0xffffffff00000000, buffer->cursor & 0x00000000ffffffff);
    mvwprintw(window, 0, 72 / 2 - ((strlen(name)) / 2), name);
    wmove(window, 1, 2);

    // If a comment is present at this address, fill the field with it and move the cursor
    // to the end.
    //
    const char *comment;
    if ((comment = buffer_lookup_comment(buffer, buffer->cursor)) != NULL) {
        set_field_buffer(fields[0], 0, comment);
        while (*comment++ != '\0') {
            form_driver(form, REQ_NEXT_CHAR);
        }
    }

    refresh();
    wrefresh(window);

    state->window = window;
    state->form = form;
    state->buffer = buffer;

    dialog_t *dialog = malloc(sizeof(dialog_t));
    dialog->driver = comments_driver;
    dialog->unpost = comments_unpost;
    dialog->user_data = (void*) state;

    // Set the cursor to visible.
    //
    curs_set(1);

    return dialog;
}
