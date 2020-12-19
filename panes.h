#pragma once

#include "render.h"
#include "buffer.h"

// Pointer into a buffer_t, used to maintain buffer cursor positioning.
// MAY NOT point outside of a buffer.
//
typedef uintptr_t cursor_t;

typedef enum {
    PANE_HEX,
    PANE_TEXT,
} pane_type_t;

typedef struct {
    pane_type_t type;
    void (*driver)(void *user_data, int input);
    void (*unpost)(void *user_data);
    void (*scroll)(void *user_data, uint64_t offset);
    const options_t *options;
    void *user_data;
} pane_t;

void pane_drive(pane_t *pane, int input);
void pane_unpost(pane_t *pane);
// Scroll to a PHYSICAL offset.
//
void pane_scroll(pane_t *pane, uint64_t offset);

extern const options_t HEX_OPT;
pane_t *hex_post(buffer_t *buffer, int width, int height);

extern const options_t TEXT_OPT;
pane_t *text_post(buffer_t *buffer, int width, int height);
