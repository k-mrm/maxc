#ifndef MAXC_MEM_H
#define MAXC_MEM_H

#include "maxc.h"
#include "object/object.h"

#define OBJECT_POOL

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

typedef struct ObjectPool {
    MxcObject **pool;
    uint16_t len;
    uint16_t reserved;
} ObjectPool;

#define OBPOOL_LAST (obpool.pool[obpool.len - 1])

void New_Objectpool();
void obpool_push(MxcObject *);

#endif

// reference counter
#define INCREF(ob) (++((MxcObject *)(ob))->refcount)

#ifdef OBJECT_POOL

#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--((MxcObject *)(ob))->refcount == 0) {                             \
            obpool_push((MxcObject *)(ob));                                    \
        }                                                                      \
    } while(0)
#else
#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--((MxcObject *)(ob))->refcount == 0) {                             \
            free(ob);                                                          \
        }                                                                      \
    } while(0)
#endif

MxcObject *Mxc_malloc(size_t);

#endif
