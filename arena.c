#include "arena.h"
#include <assert.h>
#include <stddef.h>

Arena* ArenaAlloc(uint64_t capacity)
{
    Arena* arena         = malloc(capacity + offsetof(Arena, data));
    arena->capacity      = capacity;
    arena->size          = 0;
    arena->autoalignment = 0;
    arena->data          = (uint64_t)&arena->data;
    return arena;
}
void ArenaRelease(Arena* arena) { free(arena); }
void ArenaSetAutoAlign(Arena* arena, uint64_t align)
{
    assert((align & 0x7) == 0);
    arena->autoalignment = align;
}

uint64_t ArenaPos(Arena* arena) { return arena->data; }

void* ArenaPushNoZero(Arena* arena, uint64_t size)
{
    void* ptr = (void*)arena->data;
    // Push ignoring autoalignment
    if (arena->size + size > arena->capacity)
        return NULL;
    arena->size += size;
    arena->data += size;
    return ptr;
}
void* ArenaPushAligner(Arena* arena, uint64_t aligner)
{
    assert((aligner & 0x7) == 0);

    uint64_t inc = (aligner - (arena->data % aligner)) % aligner;

    if (arena->size + inc > arena->capacity)
        return NULL;

    arena->size += inc;
    arena->data += inc;
    return (void*)arena->data;
}
void* ArenaPush(Arena* arena, uint64_t size)
{
    ArenaPushAligner(arena, arena->autoalignment);
    return ArenaPushNoZero(arena, size);
}

void ArenaPopTo(Arena* arena, uint64_t pos)
{
    size_t amt = 0;
    if (pos < arena->data)
        amt = arena->data - pos;
    ArenaPop(arena, amt);
}

void ArenaPop(Arena* arena, uint64_t size)
{
    if (size > arena->size)
        size = arena->size;
    arena->size -= size;
    arena->data -= size;
}

void ArenaClear(Arena* arena) { ArenaPop(arena, arena->size); }
