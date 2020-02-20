#ifndef MXC_GC_H
#define MXC_GC_H

#include "mem.h"

typedef struct GCHeap {
    struct GCHeap *next;
    MxcObject *obj;
} GCHeap;

extern GCHeap root;
extern GCHeap *tailp;

void dump_heap(void);

#endif
