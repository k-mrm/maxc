#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "frame.h"
#include "util.h"

extern Vector *ltable;
extern int error_flag;

int vm_run(MContext *);
int vm_exec(MContext *);
void stack_dump(void);

#define Push(ob) (*frame->stackptr++ = (ob))
#define Pop() (*--frame->stackptr)
#define Top() (frame->stackptr[-1])
#define SetTop(ob) (frame->stackptr[-1] = (ob))

#endif
