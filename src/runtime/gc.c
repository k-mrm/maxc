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
    MxcValue *base = cur_frame->stackbase;
    MxcValue *cur = cur_frame->stackptr;
    MxcValue val;
    puts("---stackweak---");
    while(base < cur) {
        val = *--cur;
        if(isobj(val)) {
            printf("%p:", optr(val));
            printf("%p\n", OBJIMPL(optr(val)));
        }
    }
    puts("---------------");
}

static void gc_mark_all() {
    MxcValue *base = cur_frame->stackbase;
    MxcValue *cur = cur_frame->stackptr;
    MxcValue val;
    while(base < cur) {
        val = *--cur;
        mgc_mark(val);
    }
    for(size_t i = 0; i < cur_frame->ngvars; ++i) {
        val = cur_frame->gvars[i];
        mgc_mark(val);
    }

    Frame *f = cur_frame;
    while(f) {
        for(size_t i = 0; i < f->nlvars; ++i) {
            val = f->lvars[i];
            mgc_mark(val);
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
        if(ob->marked || ob->gc_guard) {
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
    /*
    size_t before = heap_length(); */
    clock_t start, end;

    start = clock();

    gc_mark_all();
    gc_sweep();

    end = clock();

    gc_time += end - start;
    /*
    size_t after = heap_length();
    printf("before: %zdslots, after: %zdslots\n", before, after); */
}
