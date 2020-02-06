#ifndef MAXC_MEM_H
#define MAXC_MEM_H

#include "object/object.h"
#include "object/boolobject.h"
#include "object/charobject.h"
#include "object/floatobject.h"
#include "object/funcobject.h"
#include "object/intobject.h"
#include "object/listobject.h"
#include "object/nullobject.h"
#include "object/strobject.h"

#define OBJECT_POOL

extern size_t used_mem;

#ifdef OBJECT_POOL
union obalign {
    IntObject i;
    FloatObject fl;
    BoolObject b;
    CharObject c;
    ListObject l;
    StructObject st;
    StringObject s;
    TupleObject t;
    FunctionObject fn;
    BltinFuncObject bf;
};

typedef struct ObjectPool {
    MxcObject **pool;
    uint16_t len;
    uint16_t reserved;
} ObjectPool;

#define OBPOOL_LAST (obpool.pool[obpool.len - 1])

void New_Objectpool(void);
void obpool_push(MxcObject *);

#endif

/* Mxc_free */
#ifdef OBJECT_POOL
#   define Mxc_free(ob) do {                    \
        obpool_push((MxcObject *)(ob));         \
    } while(0)
#else
#   define Mxc_free(ob) free(ob)
#endif  /* OBJECT_POOL */

MxcObject *Mxc_malloc(size_t);

#endif
