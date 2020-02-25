#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "object/object.h"
#include "gc.h"
#include "vm.h"

GCHeap root;
GCHeap *tailp = NULL;

clock_t gc_time;

void heap_dump() {
    GCHeap *ptr = &root;
    int counter = 0;
    puts("----- [heap dump] -----");
    while(ptr) {
        bool marked = ptr->obj->marked;
        printf("%s%d: ", marked ? "[marked]" : "", counter++);
        printf("%s\n", OBJIMPL(ptr->obj)->type_name);
        ptr = ptr->next;
    }
    puts("-----------------------");
}

size_t heap_length() {
    GCHeap *ptr = &root;
    size_t i = 0;
    while(ptr) {
        ptr = ptr->next;
        ++i;
    }
    return i;
}

void stack_dump_weak() {
    MxcObject **top = cur_frame->stacktop;
    MxcObject **cur = cur_frame->stackptr;
    MxcObject *ob;
    puts("---stackweak---");
    while(top < cur) {
        ob = *--cur;
        printf("%p:", ob);
        printf("%p\n", OBJIMPL(ob));
    }
    puts("---------------");
}

static void gc_mark() {
    MxcObject **top = cur_frame->stacktop;
    MxcObject **cur = cur_frame->stackptr;
    MxcObject *ob;
    while(top < cur) {
        ob = *--cur;
        OBJIMPL(ob)->mark(ob);
    }
    for(size_t i = 0; i < cur_frame->ngvars; ++i) {
        ob = cur_frame->gvars[i];
        if(ob) OBJIMPL(ob)->mark(ob);
    }

    Frame *f = cur_frame;
    while(f) {
        for(size_t i = 0; i < f->nlvars; ++i) {
            ob = f->lvars[i];
            if(ob) OBJIMPL(ob)->mark(ob);
        }
        f = f->prev;
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
    /*size_t before = heap_length();*/
    clock_t start, end;

    start = clock();
    gc_mark();
    gc_sweep();
    end = clock();

    gc_time += end - start;
    /*
    size_t after = heap_length();
    printf("before: %zdbyte after: %zdbyte\n", before, after);
    stack_dump(); */
}
