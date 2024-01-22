#include "arena.h"

#include <assert.h>

Arena* ArenaAlloc(uint64_t capacity)
{
    assert(capacity > sizeof(Arena));
    Arena* arena         = malloc(capacity);
    arena->capacity      = capacity;
    arena->size          = ARENA_MIN_SIZE;
    arena->autoalignment = 0;
    arena->prev          = NULL;
    arena->next          = NULL;
    arena->data          = (uint64_t)&arena->data;
    return arena;
}
void ArenaRelease(Arena* arena)
{
    while (arena->prev != NULL)
        arena = arena->prev;

    Arena* next = arena->next;
    while (arena != NULL)
    {
        free(arena);
        arena = next;
        if (arena)
            next = arena->next;
    }
}
void ArenaSetAutoAlign(Arena* arena, uint64_t align)
{
    assert((align & 0x7) == 0);

    while (arena != NULL)
    {
        arena->autoalignment = align;
        arena                = arena->next;
    }
}

uint64_t ArenaPos(Arena* arena) { return arena->data; }

Arena* GetArenaWithCapacity(Arena* arena, size_t size)
{
    while ((arena->size + size) > arena->capacity)
    {
        if (arena->next == NULL)
        {
            size_t amt = arena->capacity;
            if (size > amt)
                amt = size;
            amt                        *= 2;
            arena->next                = ArenaAlloc(amt);
            arena->next->autoalignment = arena->autoalignment;
            arena->next->prev          = arena;

            ArenaPushAligner(arena->next, arena->next->autoalignment);

            // To avoid distributing small amounts of memory BEFORE the tail block,
            // we will fill the remaining space in the current subblock.
            // This guarantees we always distribute chunks from the tail
            size_t bump_remaining = arena->capacity - arena->size;
            arena->size           += bump_remaining;
            arena->data           += bump_remaining;
        }
        arena = arena->next;
    }
    return arena;
}

void* ArenaPushNoZero(Arena* arena, uint64_t size)
{
    assert(arena->prev == NULL); // first in chain

    arena = GetArenaWithCapacity(arena, size);

    void* ptr   = (void*)arena->data;
    arena->size += size;
    arena->data += size;
    return ptr;
}
void* ArenaPushAligner(Arena* arena, uint64_t aligner)
{
    assert((aligner & 0x7) == 0);

    uint64_t inc = (aligner - (arena->data % aligner)) % aligner;

    arena = GetArenaWithCapacity(arena, inc);

    arena->size += inc;
    arena->data += inc;
    return (void*)arena->data;
}
void* ArenaPush(Arena* arena, uint64_t size)
{
    assert(arena->prev == NULL); // first in chain
    ArenaPushAligner(arena, arena->autoalignment);
    return ArenaPushNoZero(arena, size);
}

void ArenaPopTo(Arena* arena, uint64_t pos)
{
    assert(arena->prev == NULL); // first in chain

    uint64_t begin = (uint64_t)&arena->data;
    uint64_t end   = (uint64_t)&arena->data + arena->capacity;
    while (pos < begin && pos > end)
    {
        arena = arena->next;
        if (arena == NULL)
            return;

        begin = (uint64_t)&arena->data;
        end   = (uint64_t)&arena->data + arena->capacity;
    }

    if (arena->next)
        ArenaClear(arena->next);

    size_t amt = 0;
    if (pos < arena->data)
        amt = arena->data - pos;
    ArenaPop(arena, amt);
}

void ArenaPop(Arena* arena, uint64_t size)
{
    while (arena->next != NULL)
        arena = arena->next;

    while (size > 0)
    {
        size_t amt = size;
        if (amt > (arena->size - ARENA_MIN_SIZE))
            amt = (arena->size - ARENA_MIN_SIZE);

        arena->size -= amt;
        arena->data -= amt;
        size        -= amt;

        if (arena->size == ARENA_MIN_SIZE)
            arena = arena->prev;
        if (arena == NULL)
            break;
    }
}

void ArenaClear(Arena* arena)
{
    // This does not require being the first item in a linked chain
    while (arena != NULL)
    {
        arena->size = ARENA_MIN_SIZE;
        arena->data = (uint64_t)&arena->data;
        arena       = arena->next;
    }
}
