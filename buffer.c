#include "buffer.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void
buffer_from_data(buffer_t *buffer, const uint8_t *data, size_t size)
{
    buffer->data = (uint8_t*) data;
    buffer->size = size;
    buffer->path = NULL;
    buffer->f = -1;
    buffer->start_mark = -1;
    buffer->end_mark = -1;
    buffer->cursor = 0;
    buffer->comments = g_hash_table_new_full(NULL, NULL, NULL, g_free);
    buffer->highlights = NULL;
    buffer->bookmarks_head = -1;
    buffer->editable = 0;
}

int
buffer_open(buffer_t *buffer, const char *path)
{
    int f = 0;
    struct stat status = {};
    uint8_t *data = NULL;

    f = open(path, O_RDONLY);
    if (f < 0) {
        perror("open");
        goto error;
    }

    buffer->f = f;

    if (fstat(f, &status) < 0) {
        perror("fstat");
        goto error;
    }

    buffer->size = status.st_size;

    data = mmap(NULL, status.st_size, PROT_READ, MAP_PRIVATE, f, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        goto error;
    }

    buffer->data = data;
    buffer->path = strdup(path);
    buffer->start_mark = -1;
    buffer->end_mark = -1;
    buffer->cursor = 0;
    buffer->comments = g_hash_table_new_full(NULL, NULL, NULL, g_free);
    buffer->highlights = NULL;
    buffer->bookmarks_head = -1;
    buffer->editable = 0;
    return 0;
error:
    if (f > 0) {
        close(f);
    }

    return 1;
}

int
buffer_close(buffer_t *buffer)
{
    if (buffer->f < 0) {
        g_hash_table_unref(buffer->comments);
        if (buffer->highlights) {
            g_slist_free_full(buffer->highlights, g_free);
        }
        return 0;
    }

    int status = 0;

    if (munmap(buffer->data, buffer->size) != 0) {
        perror("munmap");
        status = 1;
    }

    if (close(buffer->f) != 0) {
        perror("close");
        status = 1;
    }

    g_hash_table_unref(buffer->comments);
    if (buffer->highlights) {
        g_slist_free_full(buffer->highlights, g_free);
    }
    free((void*) buffer->path);
    return status;
}

int
buffer_try_reopen(buffer_t *buffer)
{
    // Not possible to reopen a in-memory buffer.
    //
    if (buffer->path == NULL) {
        return 1;
    }

    // See if the file can be opened as writable.
    //
    int f = open(buffer->path, O_RDWR);
    if (f < 0) {
        return 1;
    }

    struct stat status = {};
    if (fstat(f, &status) < 0) {
        close(f);
        return 1;
    }

    uint8_t *data = mmap(NULL, status.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
    if (data == MAP_FAILED) {
        close(f);
        return 1;
    }

    (void) munmap(buffer->data, buffer->size);
    (void) close(buffer->f);

    buffer->data = data;
    buffer->f = f;
    buffer->size = status.st_size;
    buffer->editable = 1;
    return 0;
}

int
buffer_read(buffer_t *buffer, void *data, size_t size)
{
    if (buffer->cursor + size > buffer->size) {
        return 1;
    }

    memcpy(data, &buffer->data[buffer->cursor], size);
    return 0;
}

int
buffer_read_u8(buffer_t *buffer, uint8_t *data)
{
    return buffer_read(buffer, data, sizeof(*data));
}

int
buffer_read_i8(buffer_t *buffer, int8_t *data)
{
    return buffer_read(buffer, data, sizeof(*data));
}

int
buffer_read_lu16(buffer_t *buffer, uint16_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint16_t) v[0] | (uint16_t) v[1] << 8;
    return 0;
}

int
buffer_read_bu16(buffer_t *buffer, uint16_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint16_t) v[1] | (uint16_t) v[0] << 8;
    return 0;
}

int
buffer_read_lu32(buffer_t *buffer, uint32_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint32_t) v[0] | (uint32_t) v[1] << 8 | (uint32_t) v[2] << 16 | (uint32_t) v[3] << 24;
    return 0;
}

int
buffer_read_bu32(buffer_t *buffer, uint32_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint32_t) v[3] | (uint32_t) v[2] << 8 | (uint32_t) v[1] << 16 | (uint32_t) v[0] << 24;
    return 0;
}

int
buffer_read_lu64(buffer_t *buffer, uint64_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint64_t) v[0] | (uint64_t) v[1] << 8 | (uint64_t) v[2] << 16 | (uint64_t) v[3] << 24 | (uint64_t) v[4] << 32 | (uint64_t) v[5] << 40 | (uint64_t) v[6] << 48 | (uint64_t) v[7] << 56;
    return 0;
}

int
buffer_read_bu64(buffer_t *buffer, uint64_t *data)
{
    uint8_t v[sizeof(*data)];
    if (buffer_read(buffer, v, sizeof(v))) {
        return 1;
    }

    *data = (uint64_t) v[7] | (uint64_t) v[6] << 8 | (uint64_t) v[5] << 16 | (uint64_t) v[4] << 24 | (uint64_t) v[3] << 32 | (uint64_t) v[2] << 40 | (uint64_t) v[1] << 48 | (uint64_t) v[0] << 56;
    return 0;
}

int
buffer_read_li16(buffer_t *buffer, int16_t *data)
{
    return buffer_read_lu16(buffer, (uint16_t*) data);
}

int
buffer_read_bi16(buffer_t *buffer, int16_t *data)
{
    return buffer_read_bu16(buffer, (uint16_t*) data);
}

int
buffer_read_li32(buffer_t *buffer, int32_t *data)
{
    return buffer_read_lu32(buffer, (uint32_t*) data);
}

int
buffer_read_bi32(buffer_t *buffer, int32_t *data)
{
    return buffer_read_bu32(buffer, (uint32_t*) data);
}

int
buffer_read_li64(buffer_t *buffer, int64_t *data)
{
    return buffer_read_lu64(buffer, (uint64_t*) data);
}

int
buffer_read_bi64(buffer_t *buffer, int64_t *data)
{
    return buffer_read_bu64(buffer, (uint64_t*) data);
}

uint64_t
buffer_scroll(buffer_t *buffer, uint64_t offset, int width, int height)
{
    uint64_t rows = height - 2;
    uint64_t middle = rows / 2;

    uint64_t start = offset - (offset % width);
    int64_t scroll = start / width - middle;

    if (scroll < 0) {
        scroll = 0;
    }

    if (scroll > buffer->size / width - height + 3) {
        scroll = buffer->size / width - height + 3;
    }

    if (offset > buffer->size - 1) {
        offset = buffer->size - 1;
    }

    buffer->cursor = offset;
    return scroll;
}

void
buffer_add_comment(buffer_t *buffer, uintptr_t address, char *message)
{
    g_hash_table_insert(buffer->comments, GSIZE_TO_POINTER(address), g_strdup(message));
}

void
buffer_remove_comment(buffer_t *buffer, uintptr_t address)
{
    g_hash_table_remove(buffer->comments, GSIZE_TO_POINTER(address));
}

const char*
buffer_lookup_comment(buffer_t *buffer, uintptr_t address)
{
    return g_hash_table_lookup(buffer->comments, GSIZE_TO_POINTER(address));
}

void
buffer_highlight_range(buffer_t *buffer, uintptr_t address, uint32_t size, uint32_t color)
{
    range_t *range = g_malloc(sizeof(range_t));
    range->address = address;
    range->size = size;
    range->color = color;
    buffer->highlights = g_slist_append(buffer->highlights, range);
}

void
buffer_bookmark_push(buffer_t *buffer, uintptr_t address)
{
    if (buffer->bookmarks_head < BOOKMARK_STACK_SIZE - 1) {
        buffer->bookmarks[++buffer->bookmarks_head] = address;
    }
}

uint64_t
buffer_bookmark_pop(buffer_t *buffer, int width, int height, int *error)
{
    if (buffer->bookmarks_head >= 0) {
        uintptr_t address = buffer->bookmarks[buffer->bookmarks_head--];
        *error = 0;
        return buffer_scroll(buffer, address, width, height);
    }

    *error = 1;
    return 0;
}
