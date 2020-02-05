#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "frame.h"

typedef void (*debug_fn)(Frame *);

void start_debug(Frame *);
void stack_trace(Frame *);
void local_vars(Frame *);
void debug_help(Frame *);

#endif
