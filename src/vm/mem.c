#include <unistd.h>
#include "mem.h"

#ifdef OBJECT_POOL
ObjectPool obpool;
size_t used_mem;

void New_Objectpool() {
    obpool.pool = malloc(sizeof(MxcObject *) * 512);
    obpool.reserved = obpool.len = 512;

    for(int i = 0; i < 512; ++i) {
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

#endif

MxcObject *Mxc_malloc(size_t s) {
#ifdef OBJECT_POOL
    if(obpool.len == 0) {
        New_Objectpool();
    }
    used_mem += sizeof(union obalign);
    printf("%ld\n", used_mem);

    MxcObject *ob = obpool_pop();
#else
    MxcObject *ob = (MxcObject *)malloc(s);
#endif
    ob->refcount = 1;

    return ob;
}

