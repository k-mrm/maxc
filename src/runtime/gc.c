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
}

static void gc_sweep() {
    GCHeap *ptr = &root;
    GCHeap *prev = NULL;
    MxcObject *ob;
    while(ptr) {
        ob = ptr->obj;
        if(ob->marked) {
            ob->marked = 0;
        }
        else {
            OBJIMPL(ob)->dealloc(ob);
            if(prev) prev->next = ptr->next;
            else root = *ptr->next;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}

void gc_run() {
    gc_mark();
    gc_sweep();
}
