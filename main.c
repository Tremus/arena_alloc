#include "arena.h"
#include <assert.h>

static const uint64_t ALIGNMENT = 32;

int main()
{
    Arena* a = ArenaAlloc(1024);

    ArenaSetAutoAlign(a, ALIGNMENT);

    uint64_t ptr = (uint64_t)ArenaPush(a, ALIGNMENT * 4 - 8); // offset by -8
    assert(((uint64_t)ptr % ALIGNMENT) == 0);
    uint64_t pos = ArenaPos(a);
    assert((pos % ALIGNMENT) == (ALIGNMENT - 8));

    ArenaPopTo(a, pos - 2);
    pos = ArenaPos(a);
    assert((pos % ALIGNMENT) == (ALIGNMENT - 10));

    ArenaPop(a, (1 << 16));
    assert(a->size == ARENA_MIN_SIZE);
    pos = ArenaPos(a);
    assert((uint64_t)&a->data == pos);

    ArenaClear(a);
    assert(a->size == ARENA_MIN_SIZE);

    // check chaining
    ArenaPush(a, 4096);
    assert(a->next != NULL);
    assert(a->size == a->capacity);

    ArenaPop(a, 4090);
    pos = ArenaPos(a->next);
    assert((pos % a->autoalignment) == 6);

    ArenaPopTo(a, (uint64_t)&a->data + a->capacity / 2);
    assert(a->next->size == ARENA_MIN_SIZE);
    assert(a->next->data == (uint64_t)&a->next->data);
    pos = ArenaPos(a);
    assert(pos == (uint64_t)&a->data + a->capacity / 2);

    ArenaPopTo(a, (uint64_t)a);
    assert(a->size == ARENA_MIN_SIZE);
    assert(a->next->size == ARENA_MIN_SIZE);

    ArenaRelease(a);
    return 0;
}