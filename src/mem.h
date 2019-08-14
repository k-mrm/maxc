#ifndef MAXC_MEM_H
#define MAXC_MEM_H

#include "maxc.h"
#include "object.h"

#define OBJECT_POOL

struct MxcObject;

#ifdef OBJECT_POOL

union obalign {
    IntObject i;
    FloatObject fl;
    BoolObject b;
    CharObject c;
    ListObject l;
    StringObject s;
    TupleObject t;
    FunctionObject fn;
    BltinFuncObject bf;
    StructObject st;
};

class ObjectPool {
  public:
    std::vector<MxcObject *> pool;

    void realloc();

    ObjectPool() {
        pool.resize(100);

        for(int i = 0; i < 100; ++i) {
            pool[i] = (MxcObject *)malloc(sizeof(obalign));
        }
    }
};

#endif

// reference counter
#define INCREF(ob) (++(ob)->refcount)

#ifdef OBJECT_POOL
extern ObjectPool obpool;

#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--ob->refcount == 0) {                                              \
            obpool.pool.push_back(ob);                                         \
        }                                                                      \
    } while(0)
#else
#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--ob->refcount == 0) {                                              \
            free(ob);                                                          \
        }                                                                      \
    } while(0)
#endif

MxcObject *Mxc_malloc(size_t);

#endif
