#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "frame.h"
#include "util.h"

extern Vector *ltable;
extern int error_flag;

int VM_run(Frame *);
int vm_exec(Frame *);
void stack_dump(void);

#define Push(ob) (*frame->stackptr++ = (MxcObject *)(ob))
#define Pop() (*--frame->stackptr)
#define Top() (frame->stackptr[-1])
#define SetTop(ob) (frame->stackptr[-1] = ((MxcObject *)(ob)))


#endif
