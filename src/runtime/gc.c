#include <stdlib.h>

#include "object/object.h"
#include "gc.h"

GCHeap root;
GCHeap *tailp = NULL;

void dump_heap() {
    GCHeap *ptr = &root;
    int counter = 0;
    puts("----- [heap dump] -----");
    while(ptr) {
        printf("%d: ", counter++);
        printf("%p\n", ptr->obj);
        ptr = ptr->next;
    }
}

static void gc_mark() {
    MxcObject **top = cur_frame->stacktop;
    MxcObject **cur = cur_frame->stackptr;
    MxcObject *ob;
    while(top > cur) {
        ob = *--top;
        OBJIMPL(ob)->mark(ob);
    }
    for(int i = 0; i < cur_frame->ngvars; ++i) {
        ob = cur_frame->gvars[i];
        if(ob) OBJIMPL(ob)->mark(ob);
    }

    for(int i = 0; i < cur_frame->nlvars; ++i) {
        ob = cur_frame->lvars[i];
        if(ob) OBJIMPL(ob)->mark(ob);
    }
}

static void gc_sweep() {
    GCHeap *ptr = &root;
    GCHeap *prev = NULL;
    GCHeap *next = NULL;
    MxcObject *ob;
    while(ptr) {
        ob = ptr->obj;
        next = ptr->next;
        if(ob->marked) {
            ob->marked = 0;
            prev = ptr;
        }
        else {
            OBJIMPL(ob)->dealloc(ob);
            if(prev) prev->next = ptr->next;
            else root = *ptr->next;
            free(ptr);
        }
        ptr = next;
    }

    tailp = prev;
}

void gc_run() {
    gc_mark();
    gc_sweep();
}
