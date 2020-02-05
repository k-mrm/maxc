#ifndef MAXC_VM_H
#define MAXC_VM_H

#include "frame.h"
#include "util.h"

extern Vector *ltable;
extern int error_flag;

int VM_run(Frame *);
int vm_exec(Frame *);

#endif
