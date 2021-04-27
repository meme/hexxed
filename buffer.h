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
    // If a mark is set, both marks != -1. Otherwise, the end mark is set to the cursor
    // position.
    //
    cursor_t start_mark;
    cursor_t end_mark;
    cursor_t cursor;

    GHashTable *comments;
    GSList *highlights;
    uintptr_t bookmarks[BOOKMARK_STACK_SIZE];
    int bookmarks_head;
    int editable;
} buffer_t;

typedef struct {
    uintptr_t address;
    uint32_t size;
    uint32_t color;
} range_t;

void buffer_from_data(buffer_t *buffer, const uint8_t *data, size_t size);
int buffer_open(buffer_t *buffer, const char *path);
int buffer_close(buffer_t *buffer);
// Attempt to reopen the current buffer as read-write, if it fails, the current
// buffer is left intact. If successful, all pointers into the previous map are
// invalidated.
//
int buffer_try_reopen(buffer_t *buffer);

int buffer_read(buffer_t *buffer, void *data, size_t size);
int buffer_read_u8(buffer_t *buffer, uint8_t *data);
int buffer_read_i8(buffer_t *buffer, int8_t *data);
int buffer_read_lu16(buffer_t *buffer, uint16_t *data);
int buffer_read_bu16(buffer_t *buffer, uint16_t *data);
int buffer_read_lu32(buffer_t *buffer, uint32_t *data);
int buffer_read_bu32(buffer_t *buffer, uint32_t *data);
int buffer_read_lu64(buffer_t *buffer, uint64_t *data);
int buffer_read_bu64(buffer_t *buffer, uint64_t *data);
int buffer_read_li16(buffer_t *buffer, int16_t *data);
int buffer_read_bi16(buffer_t *buffer, int16_t *data);
int buffer_read_li32(buffer_t *buffer, int32_t *data);
int buffer_read_bi32(buffer_t *buffer, int32_t *data);
int buffer_read_li64(buffer_t *buffer, int64_t *data);
int buffer_read_bi64(buffer_t *buffer, int64_t *data);

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
