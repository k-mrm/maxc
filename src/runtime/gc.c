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
    bool marked = OBJGCMARKED(ptr->obj);
    printf("%s%d: ", marked ? "[marked]" : "", counter++);
    printf("%s\n", SYSTEM(ptr->obj)->type_name);
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
  VM *vm = curvm();
  MxcValue *base = vm->stackbase;
  MxcValue *cur = vm->stackptr;
  MxcValue val;
  puts("---stackweak---");
  while(base < cur) {
    val = *--cur;
    if(isobj(val)) {
      printf("%p:", V2O(val));
      printf("%p\n", SYSTEM(V2O(val)));
    }
  }
  puts("---------------");
}

static void gc_mark_all() {
  VM *vm = curvm();
  MxcValue *base = vm->stackbase;
  MxcValue *cur = vm->stackptr;
  MxcValue val;
  while(base < cur) {
    val = *--cur;
    mgc_mark(val);
  }
  for(size_t i = 0; i < vm->ngvars; ++i) {
    val = vm->gvars[i];
    mgc_mark(val);
  }

  MContext *c = vm->ctx;
  while(c) {
    for(size_t i = 0; i < c->nlvars; i++) {
      val = c->lvars[i];
      mgc_mark(val);
    }
    c = c->prev;
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
    if(OBJGCMARKED(ob) || OBJGCGUARDED(ob)) {
      OBJGCUNMARK(ob);
      prev = ptr;
    }
    else {
      SYSTEM(ob)->dealloc(ob);
      if(prev) prev->next = ptr->next;
      else root = *ptr->next;
      free(ptr);
    }
    ptr = next;
  }

  tailp = prev;
}

void gc_run() {
  clock_t start, end;
  start = clock();

  gc_mark_all();
  gc_sweep();

  end = clock();
  gc_time += end - start;
}
