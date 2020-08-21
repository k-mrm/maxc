#ifndef MXC_GC_H
#define MXC_GC_H

#include "mem.h"
#include "context.h"

extern MContext *cur_frame;

typedef struct GCHeap {
  struct GCHeap *next;
  MxcObject *obj;
} GCHeap;

extern GCHeap root;
extern GCHeap *tailp;

void heap_dump(void);
size_t heap_length(void);
void gc_run(void);

#endif
