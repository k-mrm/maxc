#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "maxc.h"
#include "vm.h"
#include "frame.h"
#include "function.h"
#include "object/object.h"

typedef void (*debug_fn)(Frame *);

void start_debug(Frame *);
void stack_trace(Frame *frame);

#endif
