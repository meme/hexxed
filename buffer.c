#include "buffer.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int
buffer_open(buffer_t *buffer, const char *path)
{
    int f = 0;
    struct stat status = {};
    uint8_t *data = NULL;

    f = open(path, O_RDWR);
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

    data = mmap(NULL, status.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, f, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        goto error;
    }

    buffer->data = data;
    buffer->path = strdup(path);
    buffer->mark = -1;
    buffer->cursor = 0;
    buffer->comments = g_hash_table_new_full(NULL, NULL, NULL, g_free);
    buffer->highlights = NULL;
    buffer->bookmarks_head = -1;
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
    if (buffer->highlights)
        g_slist_free_full(buffer->highlights, g_free);
    free(buffer->path);
    return status;
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
