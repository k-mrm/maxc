#ifndef MXC_GC_H
#define MXC_GC_H

#include "mem.h"
#include "frame.h"

extern Frame *cur_frame;

typedef struct GCHeap {
    struct GCHeap *next;
    MxcObject *obj;
} GCHeap;

extern GCHeap root;
extern GCHeap *tailp;

void dump_heap(void);
void gc_run(void);

#endif
