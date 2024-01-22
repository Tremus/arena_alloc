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
    assert(a->size == 0);
    pos = ArenaPos(a);
    assert((uint64_t)&a->data == pos);

    ArenaRelease(a);
    return 0;
}