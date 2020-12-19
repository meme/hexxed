#pragma once

#include <stdint.h>
#include <stddef.h>
#include <gmodule.h>

#define BOOKMARK_STACK_SIZE 8

typedef uintptr_t cursor_t;

typedef struct {
    size_t size;
    uint8_t *data;
    int f;
    const char *path;
    // If a mark is set, mark != -1. Otherwise, the mark is set to the cursor
    // position.
    //
    cursor_t mark;
    cursor_t cursor;

    GHashTable *comments;
    GSList *highlights;
    uintptr_t bookmarks[BOOKMARK_STACK_SIZE];
    int bookmarks_head;
} buffer_t;

typedef struct {
    uintptr_t address;
    uint32_t size;
    uint32_t color;
} range_t;

int buffer_open(buffer_t *buffer, const char *path);
int buffer_close(buffer_t *buffer);
// Returns the scroll required to centre the offset in a buffer view, and sets
// the cursor to "offset", or the start/end of the buffer if the offset is out
// of range.
//
uint64_t buffer_scroll(buffer_t *buffer, uint64_t offset, int width, int height);

void buffer_add_comment(buffer_t *buffer, uintptr_t address, char *message);
void buffer_remove_comment(buffer_t *buffer, uintptr_t address);
const char *buffer_lookup_comment(buffer_t *buffer, uintptr_t address);

// If size is 0, remove the highlight.
//
void buffer_highlight_range(buffer_t *buffer, uintptr_t address, uint32_t size, uint32_t color);

void buffer_bookmark_push(buffer_t *buffer, uintptr_t address);
uint64_t buffer_bookmark_pop(buffer_t *buffer, int width, int height, int *error);