#include "panes.h"
#include "buffer.h"
#include "render.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void
pane_drive(pane_t *pane, int input)
{
    pane->driver(pane->user_data, input);
}

void
pane_unpost(pane_t *pane)
{
    pane->unpost(pane->user_data);
    free(pane);
}

void
pane_scroll(pane_t *pane, uint64_t offset)
{
    pane->scroll(pane->user_data, offset);
}

// If a character can be printed to the screen SAFELY.
//
static inline int
can_print(char c)
{
    return (unsigned) c - 0x20 < 0x5f;
}

const options_t HEX_OPT = {
    [0 ... 9] = "      ",
    [2] = "Edit  ",
    // Handled in main driver.
    //
    [4] = "Goto  ",
    [8] = "Names ",
};

typedef struct {
    buffer_t *buffer;
    // If on the odd-end of a pair, e.g.: 0 for "[0]0" and 1 for "0[0]"
    //
    int odd;
    // If in edit mode, != 0.
    //
    int edit;
    uint64_t scroll;
} hex_pane_t;

static void
hex_unpost(void *user_data)
{
    free(user_data);
}

static void
hex_update(hex_pane_t *pane, int width, int height)
{
    attrset(COLOR_PAIR(COLOR_STANDARD));
    buffer_t *buffer = pane->buffer;

    uint8_t *data = buffer->data + pane->scroll * 16;
    cursor_t current;
    for (int i = 1; (current = data - buffer->data) < buffer->size && i < (height - 1); data += 16, i++) {
        // Determine the number of characters left in this row: 16 unless there
        // is no more data to print.
        //
        size_t size = buffer->size - current;
        if (size >= 16) {
            size = 16;
        }

        // .00000000`00000000:
        //
        size_t offset = 0, wrote;
        char addr_str[22];
        wrote = snprintf(addr_str, sizeof(addr_str), ".%08lx`%08lx:  ",
                current & 0xffffffff00000000,
                current & 0x00000000ffffffff);
        mvaddstr(i, offset, addr_str);
        offset += wrote;

        // 00 00 00 00-00 00 00 00-00 00 00 00-00 00 00 00
        //
        for (int j = 0; j < size; j++) {
            char hex_str[2];

            range_t *range = NULL;
            for (GSList *highlight = buffer->highlights; highlight != NULL; highlight = g_slist_next(highlight)) {
                range = (range_t*) highlight->data;
                uintptr_t start = range->address;
                uintptr_t end = start + range->size - 1;
                if (current >= start && current <= end) {
                    attrset(COLOR_PAIR(range->color));
                    break;
                }
            }

            // If the block is under the cursor or inside of a mark, color it.
            //
            cursor_t mark_end = buffer->end_mark == -1 ? buffer->cursor : buffer->end_mark;
            int mark_forwards = buffer->start_mark != -1 && current >= buffer->start_mark && current <= mark_end;
            int mark_backwards = buffer->start_mark != -1 && current <= buffer->start_mark && current >= mark_end;
            if (!pane->edit && (buffer->cursor == current || mark_forwards || mark_backwards)) {
                attrset(COLOR_PAIR(COLOR_SELECTED));
            }

            snprintf(hex_str, sizeof(hex_str), "%x", data[j] & 0xf0);
            if (pane->edit && !pane->odd && buffer->cursor == current) {
                attrset(COLOR_PAIR(COLOR_SELECTED));
            }
            mvaddstr(i, offset, hex_str);
            if (pane->edit && !pane->odd && buffer->cursor == current) {
                attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            snprintf(hex_str, sizeof(hex_str), "%x", data[j] & 0x0f);
            if (pane->edit && pane->odd && buffer->cursor == current) {
                attrset(COLOR_PAIR(COLOR_SELECTED));
            }
            mvaddstr(i, offset + 1, hex_str);
            if (pane->edit && pane->odd && buffer->cursor == current) {
                attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            offset += 2;

            // If there is no mark set or the cursor is on the start/end of the
            // mark or it's the last pair on the row, don't color the space.
            //
            if (mark_forwards && buffer->end_mark == current) {
                attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            if (mark_backwards && buffer->start_mark == current) {
                attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            int in_range = range != NULL && range->address + range->size - 1 == current;
            if ((buffer->cursor == current && !mark_backwards && !mark_forwards) || j == 15 || in_range) {
                attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            if (buffer->end_mark == -1 && mark_forwards && buffer->cursor == current) {
               attrset(COLOR_PAIR(COLOR_STANDARD));
            }

            current++;

            // Terminate the hex pair with a dash if it is a multiple of 4.
            //
            char end = (j + 1) % 4 == 0 && j != 15 ? '-' : ' ';
            char term_str[2] = { end, '\0' };
            mvaddstr(i, offset, term_str);
            offset += 1;

            attrset(COLOR_PAIR(COLOR_STANDARD));
        }

        current -= size;

        // If size <= 16, add more spaces.
        //
        for (int j = 0; j < 16 - size; j++) {
            // For each character in the pair, e.g.: "00 ".
            //
            mvaddstr(i, offset, "   ");
            offset += 3;
        }

        // Move the cursor over to add one more space separator. NULL is added,
        // so replace it with a space.
        //
        mvaddstr(i, offset, " ");
        offset++;

        // ................
        //
        for (int j = 0; j < size; j++) {
            char print_str[2];
            wrote = snprintf(print_str, sizeof(print_str),
                    "%c", can_print(data[j]) ? data[j] & 0xff : '.');

            cursor_t mark_end = buffer->end_mark == -1 ? buffer->cursor : buffer->end_mark;
            int mark_forwards = buffer->start_mark != -1 && current >= buffer->start_mark && current <= mark_end;
            int mark_backwards = buffer->start_mark != -1 && current <= buffer->start_mark && current >= mark_end;
            if (buffer->cursor == current || (!pane->edit && (mark_forwards || mark_backwards))) {
                attrset(COLOR_PAIR(COLOR_SELECTED));
            }

            mvaddstr(i, offset, print_str);
            offset += wrote;
            current++;

            attrset(COLOR_PAIR(COLOR_STANDARD));
        }

        // If size <= 16, add more spaces.
        //
        for (int j = 0; j < 16 - size; j++) {
            mvaddstr(i, offset, " ");
            offset++;
        }
    }

    // Convert 1D cursor coordinate to 2D.
    //
    cursor_t cursor = buffer->cursor;
    int x = strlen(".00000000`00000000: ");
    int y = (cursor / 16 - pane->scroll) + 1; // Account for the status bar

    // If there is a comment present: print the comment shifted down or up
    // depending if there is space.
    //
    const char *comment = buffer_lookup_comment(buffer, cursor);
    if (comment != NULL) {
        if (y + 5 >= height) {
            y -= 3;
        } else {
            y += 3;
        }
        attrset(COLOR_PAIR(COLOR_SELECTED));
        mvaddstr(y, x, "; ");
        mvaddstr(y, x + 2, comment);
        attrset(COLOR_PAIR(COLOR_STANDARD));
    }
}

static void
hex_driver(void *user_data, int input)
{
    hex_pane_t *pane = (hex_pane_t*) user_data;
    buffer_t *buffer = pane->buffer;

    int height, width;
    getmaxyx(stdscr, height, width);
    width = 16;

    cursor_t top = pane->scroll * width;
    cursor_t bottom = top + (height - 3) * width;

    switch (input) {
    case 'h':
    case KEY_LEFT:
        if (buffer->cursor > 0) {
            if (pane->odd == 0 || !pane->edit) {
                buffer->cursor--;
            }
            pane->odd = !pane->odd;
        } else if (buffer->cursor == 0 && pane->odd) {
            // If the user is on the second nibble of the buffer and moves to
            // the left.
            //
            pane->odd = 0;
        }

        // If moving the cursor caused it to move off the screen, overflow scroll
        // upwards.
        //
        if (buffer->cursor - (buffer->cursor % width) < (top + width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll > 0) {
                pane->scroll--;
            }
        }
        break;
    case 'j':
    case KEY_DOWN:
        if (buffer->cursor + width < buffer->size) {
            buffer->cursor += width;
        } else {
            // If on last row: snap cursor to end of the buffer.
            //
            buffer->cursor = buffer->size - 1;
        }

        // If overflow scrolled off the screen, seek downwards.
        //
        if (buffer->cursor - (buffer->cursor % width) > (bottom - width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll + height - 3 < buffer->size / width) {
                pane->scroll++;
            }
        }
        break;
advance:
    case 'l':
    case KEY_RIGHT:
        if (buffer->cursor < buffer->size - 1) {
            if (pane->odd == 1 || !pane->edit) {
                buffer->cursor++;
            }
            pane->odd = !pane->odd;
        } else if (buffer->cursor == buffer->size - 1 && !pane->odd) {
            // If the user is on the 2nd last nibble of the buffer and moves to
            // the right.
            //
            pane->odd = 1;
        }

        // If moving the cursor caused it to move off the screen, overflow scroll
        // downwards.
        //
        if (buffer->cursor - (buffer->cursor % width) > (bottom - width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll + height - 3 < buffer->size / width) {
                pane->scroll++;
            }
        }
        break;
    case 'k':
    case KEY_UP:
        if (buffer->cursor >= width) {
            buffer->cursor -= width;
        } else {
            // If on 1st row: snap cursor to the beginning of the buffer.
            //
            buffer->cursor = 0;
        }

        // If overflow scrolled off the screen, seek upwards.
        //
        if (buffer->cursor - (buffer->cursor % width) < (top + width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll > 0) {
                pane->scroll--;
            }
        }
        break;
    case KEY_PPAGE:
        // If scrolling would send the cursor off the screen, reset to the
        // beginning.
        //
        if (pane->scroll < height - 3) {
            pane->scroll = 0;
            buffer->cursor = 0;
        } else {
            pane->scroll -= height - 3;
            buffer->cursor = pane->scroll * width;
        }
        break;
    case KEY_NPAGE:
        // If scrolling would send the cursor out of bounds, set the cursor
        // to the end of the buffer.
        //
        if (pane->scroll + height - 3 >= buffer->size / width) {
            goto end;
        } else {
            pane->scroll += height - 3;
            buffer->cursor = pane->scroll * width;
        }
        break;
end:
    case KEY_END:
        pane->scroll = buffer->size / width - height + 3;
        buffer->cursor = buffer->size - 1;
        pane->odd = 1;
        break;
    case KEY_HOME:
        pane->scroll = 0;
        buffer->cursor = 0;
        pane->odd = 0;
        break;
    case 'v':
        // No selection supported inside of edit mode.
        //
        if (pane->edit)
            break;

        if (buffer->start_mark == -1) {
            buffer->start_mark = buffer->cursor;
        } else if (buffer->end_mark == -1 && buffer->start_mark != -1) {
            buffer->end_mark = buffer->cursor;
        } else if (buffer->end_mark != -1 && buffer->start_mark != -1) {
            buffer->start_mark = -1;
            buffer->end_mark = -1;
        }
        break;
    case KEY_F(3):
        if (!pane->edit) {
            pane->edit = 1;
        }
        break;
    case '\x1b': {
        // If in edit mode and ESC is hit, disable edit mode.
        //
        if (!pane->edit)
            break;

        nodelay(stdscr, TRUE);
        int n;
        if ((n = getch()) == ERR) {
            pane->edit = 0;
        } else {
            ungetch(n);
        }
        nodelay(stdscr, FALSE);
    } break;
    case 'a'...'f':
    case '0'...'9': {
        if (!pane->edit)
            break;

        // Convert character to integral representation.
        //
        int n;
        if (input >= '0' && input <= '9') {
            n = input - '0';
        } else if (input >= 'a' && input <= 'f') {
            n = 10 + input - 'a';
        }

        uint8_t b = buffer->data[buffer->cursor];
        uint8_t st = b & 0x0f;
        uint8_t nd = (b & 0xf0) >> 4;

        if (pane->odd) {
            st = n;
        } else {
            nd = n;
        }

        buffer->data[buffer->cursor] = ((nd << 4) | st);
        goto advance;
    } break;
    case '+':
        buffer_bookmark_push(buffer, buffer->cursor);
        break;
    case '-': {
        int error;
        uint64_t scroll = buffer_bookmark_pop(buffer, width, height, &error);
        if (!error) {
            pane->scroll = scroll;
        }
    } break;
    }

    hex_update(pane, width, height);
}

static void
hex_scroll(void *user_data, uint64_t offset)
{
    hex_pane_t *pane = (hex_pane_t*) user_data;

    int height, width;
    getmaxyx(stdscr, height, width);
    (void) width;

    pane->scroll = buffer_scroll(pane->buffer, offset, 16, height);
    pane->odd = 0;
}

pane_t*
hex_post(buffer_t *buffer, int width, int height)
{
    hex_pane_t *hex_pane = malloc(sizeof(hex_pane_t));
    hex_pane->buffer = buffer;
    hex_pane->odd = 0;
    hex_pane->edit = 0;
    hex_scroll((void*) hex_pane, buffer->cursor);

    pane_t *pane = malloc(sizeof(pane_t));
    pane->driver = hex_driver;
    pane->unpost = hex_unpost;
    pane->scroll = hex_scroll;
    pane->options = &HEX_OPT;
    pane->user_data = (void*) hex_pane;
    pane->type = PANE_HEX;

    hex_update(hex_pane, width, height);

    return pane;
}

const options_t TEXT_OPT = {
    [0 ... 9] = "      ",
    // Handled in main driver.
    //
    [4] = "Goto  ",
    [8] = "Names ",
};

typedef struct {
    buffer_t *buffer;
    uint64_t scroll;
} text_pane_t;

static void
text_update(text_pane_t *pane, int width, int height)
{
    attrset(COLOR_PAIR(COLOR_STANDARD));

    buffer_t *buffer = pane->buffer;

    uint8_t *data = buffer->data + pane->scroll * width;
    cursor_t current;
    for (int i = 1; (current = data - buffer->data) < buffer->size && i < (height - 1); data += width, i++) {
        size_t size = buffer->size - current;
        if (size >= width) {
            size = width;
        }

        for (int j = 0; j < size; j++) {
            // If the character cannot be SAFELY printed, print a space instead.
            //
            char c = can_print(data[j]) ? data[j] : ' ';
            // If the character is under the cursor or the current mark, color
            // it with a selection.
            //
            cursor_t mark_end = buffer->end_mark == -1 ? buffer->cursor : buffer->end_mark;
            int mark_forwards = buffer->start_mark != -1 && current >= buffer->start_mark && current <= mark_end;
            int mark_backwards = buffer->start_mark != -1 && current <= buffer->start_mark && current >= mark_end;
            if (buffer->cursor == current || mark_forwards || mark_backwards) {
                attrset(COLOR_PAIR(COLOR_SELECTED));
            }
            mvaddch(i, j, c);
            attrset(COLOR_PAIR(COLOR_STANDARD));
            current++;
        }

        // If size <= width, add more spaces.
        //
        attrset(COLOR_PAIR(COLOR_STANDARD));
        for (int j = 0; j < width - size; j++) {
            mvaddch(i, size + j, ' ');
        }
    }
}

static void
text_driver(void *user_data, int input)
{
    text_pane_t *pane = (text_pane_t*) user_data;
    buffer_t *buffer = pane->buffer;

    int height, width;
    getmaxyx(stdscr, height, width);

    cursor_t top = pane->scroll * width;
    cursor_t bottom = top + (height - 3) * width;

    switch (input) {
    case 'h':
    case KEY_LEFT:
        if (buffer->cursor > 0) {
            buffer->cursor--;
        }

        // If moving the cursor caused it to move off the screen, overflow scroll
        // upwards.
        //
        if (buffer->cursor - (buffer->cursor % width) < (top + width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll > 0) {
                pane->scroll--;
            }
        }
        break;
    case 'j':
    case KEY_DOWN:
        if (buffer->cursor + width < buffer->size) {
            buffer->cursor += width;
        } else {
            // If on last row: snap cursor to end of the buffer.
            //
            buffer->cursor = buffer->size - 1;
        }

        // If overflow scrolled off the screen, seek downwards.
        //
        if (buffer->cursor - (buffer->cursor % width) > (bottom - width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll + height - 3 < buffer->size / width) {
                pane->scroll++;
            }
        }
        break;
    case 'l':
    case KEY_RIGHT:
        if (buffer->cursor < buffer->size - 1) {
            buffer->cursor++;
        }

        // If moving the cursor caused it to move off the screen, overflow scroll
        // downwards.
        //
        if (buffer->cursor - (buffer->cursor % width) > (bottom - width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll + height - 3 < buffer->size / width) {
                pane->scroll++;
            }
        }
        break;
    case 'k':
    case KEY_UP:
        if (buffer->cursor >= width) {
            buffer->cursor -= width;
        } else {
            // If on 1st row: snap cursor to the beginning of the buffer.
            //
            buffer->cursor = 0;
        }

        // If overflow scrolled off the screen, seek upwards.
        //
        if (buffer->cursor - (buffer->cursor % width) < (top + width * 2)) {
            // If scrolling won't go off the screen.
            //
            if (pane->scroll > 0) {
                pane->scroll--;
            }
        }
        break;
    case KEY_PPAGE:
        // If scrolling would send the cursor off the screen, reset to the
        // beginning.
        //
        if (pane->scroll < height - 3) {
            pane->scroll = 0;
            buffer->cursor = 0;
        } else {
            pane->scroll -= height - 3;
            buffer->cursor = pane->scroll * width;
        }
        break;
    case KEY_NPAGE:
        // If scrolling would send the cursor out of bounds, set the cursor
        // to the end of the buffer.
        //
        if (pane->scroll + height - 3 >= buffer->size / width) {
            goto end;
        } else {
            pane->scroll += height - 3;
            buffer->cursor = pane->scroll * width;
        }
        break;
end:
    case KEY_END:
        pane->scroll = buffer->size / width - height + 3;
        buffer->cursor = buffer->size - 1;
        break;
    case KEY_HOME:
        pane->scroll = 0;
        buffer->cursor = 0;
        break;
    case 'v':
        // If there is no mark, set it to the cursor. Otherwise, clear it.
        //
        if (buffer->start_mark == -1) {
            buffer->start_mark = buffer->cursor;
        } else if (buffer->end_mark == -1 && buffer->start_mark != -1) {
            buffer->end_mark = buffer->cursor;
        } else if (buffer->end_mark != -1 && buffer->start_mark != -1) {
            buffer->start_mark = buffer->cursor;
            buffer->end_mark = -1;
        }
        break;
    }

    text_update(pane, width, height);
}

static void
text_unpost(void *user_data)
{
    free(user_data);
}

static void
text_scroll(void *user_data, uint64_t offset)
{
    text_pane_t *pane = (text_pane_t*) user_data;

    int height, width;
    getmaxyx(stdscr, height, width);

    pane->scroll = buffer_scroll(pane->buffer, offset, width, height);
}

pane_t*
text_post(buffer_t *buffer, int width, int height)
{
    text_pane_t *text_pane = malloc(sizeof(text_pane_t));
    text_pane->buffer = buffer;
    text_scroll((void*) text_pane, buffer->cursor);

    pane_t *pane = malloc(sizeof(pane_t));
    pane->driver = text_driver;
    pane->unpost = text_unpost;
    pane->scroll = text_scroll;
    pane->options = &TEXT_OPT;
    pane->user_data = text_pane;
    pane->type = PANE_TEXT;

    text_update(text_pane, width, height);

    return pane;
}
