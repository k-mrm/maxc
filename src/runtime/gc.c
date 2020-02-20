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

void gc_run() {
    ;
}
