#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "constant.h"
#include "maxc.h"
#include "object.h"

extern MxcObject **stackptr;

typedef std::vector<MxcObject *> localvar;
typedef std::vector<MxcObject *> globalvar;

class Frame {
  public:
    Frame(bytecode &b) : code(b), pc(0) {} // global

    Frame(userfunction &u) : code(u.code), pc(0) {
        lvars.resize(u.vars.get().size());
    }

    bytecode &code;
    localvar lvars;
    size_t pc;

  private:
};

class VM {
  public:
    VM(Constant &c, int ngvar) : ctable(c) { gvmap.resize(ngvar); }

    int run(bytecode &);

  private:
    Frame *frame;

    std::stack<unsigned int> locs;
    globalvar gvmap;
    Constant &ctable;

    std::stack<Frame *, std::vector<Frame *>> framestack;

    void print(MxcObject *);
    int exec();
};

// reference counter
#define INCREF(ob) (++ob->refcount)

#ifdef OBJECT_POOL
extern ObjectPool obpool;

#define DECREF(ob)  \
    do {    \
        if(--ob->refcount == 0) {   \
            obpool.pool.push_back(ob);  \
        }   \
    } while(0)
#else
#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--ob->refcount == 0) {                                              \
            free(ob);                                                          \
        }                                                                      \
    } while(0)
#endif

// stack
#define Push(ob) (*(stackptr++) = (ob))
#define Pop() (*(--stackptr))
#define Top() (stackptr[-1])
#define SetTop(ob) (stackptr[-1] = ob)

#endif
