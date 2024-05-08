typedef long i64;
typedef unsigned long u64;

struct arena {
    char *start;
    char *end;
};

void *alloc(struct arena *arena, i64 size) {
    i64 available = arena->end - arena->start;
    i64 padding = -(i64)arena->start & (16 - 1);
    if (size > (available - padding)) {
        return 0;
    }
    char *p = arena->start + padding;
    arena->start = p + size;
    for (i64 i = 0; i < size; ++i) {
        p[i] = 0;
    }
    return p;
}
