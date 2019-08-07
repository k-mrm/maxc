#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "bytecode.h"
#include "literalpool.h"
#include "maxc.h"
#include "mem.h"
#include "object.h"

extern MxcObject **stackptr;
extern LiteralPool ltable;

typedef std::vector<MxcObject *> localvar;
typedef std::vector<MxcObject *> globalvar;

struct Frame {
    Frame(uint8_t c[], size_t size) : pc(0) {
        /*
        code = (uint8_t *)malloc(sizeof(uint8_t) * size);
        printf("%d", size);
        memcpy(code, c, size);*/
        codesize = size;
        code = c;
    } // global

    Frame(userfunction u) : pc(0), nlvars(u.nlvars) {
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
    size_t pc;
    size_t nlvars;
};

class VM {
  public:
    VM(LiteralPool &l, int ngvar) {
        ltable = l;
        gvmap.resize(ngvar);
        Object::init();
    }

    int run(uint8_t[], size_t);

  private:
    Frame *frame;

    std::stack<unsigned int> locs;
    globalvar gvmap;

    std::stack<Frame *, std::vector<Frame *>> framestack;

    void print(MxcObject *);
    int exec();
};

// stack
#define Push(ob) (*stackptr++ = (ob))
#define Pop() (*--stackptr)
#define Top() (stackptr[-1])
#define SetTop(ob) (stackptr[-1] = ob)

#endif
