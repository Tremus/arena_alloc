#pragma once
#include <stdlib.h>

typedef struct Arena
{
    size_t   capacity;
    size_t   size;
    size_t   autoalignment;
    uint64_t data;
} Arena;

Arena* ArenaAlloc(uint64_t capacity);
void   ArenaRelease(Arena* arena);

// If no alignment is set, default to user set autoalignment
void ArenaSetAutoAlign(Arena* arena, uint64_t align);

uint64_t ArenaPos(Arena* arena);

// These return NULL when overflowing
void* ArenaPushNoZero(Arena* arena, uint64_t size);
void* ArenaPushAligner(Arena* arena, uint64_t aligner);
// push using user set autoalignment
void* ArenaPush(Arena* arena, uint64_t size);

void ArenaPopTo(Arena* arena, uint64_t pos);
void ArenaPop(Arena* arena, uint64_t size);
void ArenaClear(Arena* arena);
