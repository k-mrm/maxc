#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mem.h"
#include "internal.h"
#include "gc.h"

size_t allocated_mem = 0;
size_t threshold = 256;

#ifdef OBJECT_POOL
ObjectPool obpool;

#define NALLOC 256

void New_Objectpool() {
    obpool.pool = malloc(sizeof(MxcObject *) * NALLOC);
    obpool.reserved = obpool.len = NALLOC;

    for(int i = 0; i < NALLOC; ++i) {
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

#endif /* OBJECT_POOL */

MxcObject *Mxc_malloc(size_t s) {
    if(allocated_mem++ >= threshold) {
        allocated_mem = 0;
        gc_run();
    }

#ifdef OBJECT_POOL
    INTERN_UNUSE(s);
    if(obpool.len == 0) {
        New_Objectpool();
    }
    MxcObject *ob = obpool_pop();
#else
    MxcObject *ob = (MxcObject *)malloc(s);
#endif  /* OBJECT_POOL */

#ifdef USE_MARK_AND_SWEEP
    ob->marked = 0;
    if(!tailp) {    /* first call */
        root.obj = ob;
        root.next = NULL;
        tailp = &root;
    }
    else {
        GCHeap *new = malloc(sizeof(GCHeap));
        new->obj = ob;
        new->next = NULL;
        tailp->next = new;
        tailp = new;
    }
#else
    ob->refcount = 1;
#endif  /* USE_MARK_AND_SWEEP */

    return ob;
}

