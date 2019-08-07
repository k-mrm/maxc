#include "mem.h"

#ifdef OBJECT_POOL
ObjectPool obpool;

void ObjectPool::realloc() {
    pool.resize(100);
    for(int i = 0; i < 100; ++i) {
        pool[i] = (MxcObject *)malloc(sizeof(obalign));
    }
}

#endif

MxcObject *Mxc_malloc(size_t s) {
#ifdef OBJECT_POOL
    if(obpool.pool.empty()) {
        obpool.realloc();
    }

    auto ob = obpool.pool.back();
    obpool.pool.pop_back();
#else
    auto ob = (MxcObject *)malloc(s);
#endif
    ob->refcount = 1;

    return ob;
}
