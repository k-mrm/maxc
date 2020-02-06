#ifndef MAXC_OBJECT_H
#define MAXC_OBJECT_H

#include "object/objimpl.h"

#define OBJECT_HEAD MxcObject base
#define ITERABLE_OBJECT_HEAD MxcIterable base
#define ITERABLE(ob) ((MxcIterable *)(ob))

struct StringObject;
typedef struct StringObject StringObject;
typedef struct MxcObject MxcObject;
typedef struct MxcIterable MxcIterable;

typedef StringObject *(*ob_tostring_fn)(MxcObject *);
typedef void (*ob_dealloc_fn)(MxcObject *);
typedef void (*ob_mark_fn)(MxcObject *);

#define OBJIMPL(ob) (((MxcObject *)ob)->impl)

struct MxcObject {
    MxcObjImpl *impl;
    int refcount;
    /* gc: TODO */
    unsigned int marked: 1;
};

typedef struct ErrorObject {
    OBJECT_HEAD;
    const char *errmsg;
} ErrorObject;

typedef struct TupleObject {
    OBJECT_HEAD;
} TupleObject; // TODO

typedef struct StructObject {
    OBJECT_HEAD;
    MxcObject **field;
} StructObject;

StructObject *new_structobject(int);
ErrorObject *new_errorobject(const char *);

/* reference counter */
#define INCREF(ob) (++((MxcObject *)(ob))->refcount)

#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--((MxcObject *)(ob))->refcount == 0) {                             \
            OBJIMPL((MxcObject *)ob)->dealloc((MxcObject *)ob);                \
        }                                                                      \
    } while(0)

#endif
