#pragma once

#include "panes.h"
#include "render.h"

typedef struct {
    void (*driver)(void *user_data, int input);
    void (*unpost)(void *user_data);
    void *user_data;
} dialog_t;

void dialog_drive(dialog_t *dialog, int input);
void dialog_unpost(dialog_t *dialog);

extern const options_t CALCULATOR_OPT;
dialog_t *calculator_post(int top, int left);

extern const options_t GOTO_OPT;
dialog_t *goto_post(pane_t *pane, int top, int left);

dialog_t *comments_post(buffer_t *buffer, int top, int left);
