#include "mem.h"

#ifdef OBJECT_POOL
ObjectPool obpool;

void New_Objectpool() {
    obpool.pool = malloc(sizeof(MxcObject *) * 128);
    obpool.len = 128;
    obpool.reserved = 128;

    for(int i = 0; i < 128; ++i) {
        obpool.pool[i] = malloc(sizeof(union obalign));
    }
}

/* free */
void obpool_push(MxcObject *ob) {
    if(obpool.reserved == obpool.len) {
        obpool.reserved *= 2;
        obpool.pool =
            realloc(obpool.pool, sizeof(MxcObject *) * obpool.reserved);
    }

    memset(ob, 0, sizeof(union obalign));

    obpool.pool[obpool.len++] = ob;
}

static MxcObject *obpool_pop() { return obpool.pool[--obpool.len]; }

static void obpool_realloc() {
    obpool.pool = malloc(sizeof(MxcObject *) * 128);
    obpool.len = 128;
    obpool.reserved = 128;

    for(int i = 0; i < 128; ++i) {
        obpool.pool[i] = (MxcObject *)malloc(sizeof(union obalign));
    }
}

#endif

MxcObject *Mxc_malloc(size_t s) {
#ifdef OBJECT_POOL
    if(obpool.len == 0) {
        obpool_realloc();
    }

    MxcObject *ob = obpool_pop();
#else
    MxcObject *ob = (MxcObject *)malloc(s);
#endif
    ob->refcount = 1;

    return ob;
}
