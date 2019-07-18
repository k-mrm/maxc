#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "literalpool.h"
#include "maxc.h"
#include "object.h"

extern MxcObject **stackptr;
extern LiteralPool ltable;

typedef std::vector<MxcObject *> localvar;
typedef std::vector<MxcObject *> globalvar;

struct Frame {
    Frame(uint8_t c[], size_t codesize): pc(0) {
        /*
        code = (uint8_t *)malloc(sizeof(uint8_t) * size);
        printf("%d", size);
        memcpy(code, c, size);*/
        code = c;
    } // global

    Frame(userfunction u) : nlvars(u.nlvars), pc(0) {
        /*
        code = (uint8_t *)malloc(sizeof(uint8_t) * u.codelength);
        printf("%d", u.codelength);
        memcpy(code, u.code, u.codelength);*/
        code = u.code;
        lvars.resize(u.nlvars);
    }

    uint8_t *code;
    size_t codesize;
    localvar lvars;
    size_t nlvars;
    size_t pc;
};

class VM {
  public:
    VM(LiteralPool &l, int ngvar) {
        ltable = l;
        gvmap.resize(ngvar);
    }

    int run(uint8_t [], size_t);

  private:
    Frame *frame;

    std::stack<unsigned int> locs;
    globalvar gvmap;

    std::stack<Frame *, std::vector<Frame *>> framestack;

    void print(MxcObject *);
    int exec();
};

// reference counter
#define INCREF(ob) (++ob->refcount)

#ifdef OBJECT_POOL
extern ObjectPool obpool;

#define DECREF(ob)                                                             \
    do {                                                                       \
        if(--ob->refcount == 0) {                                              \
            obpool.pool.push_back(ob);                                         \
        }                                                                      \
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
