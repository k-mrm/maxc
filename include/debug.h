#ifndef MXC_DEBUG_H
#define MXC_DEBUG_H

#include "context.h"

typedef void (*debug_fn)(MContext *);

void start_debug(MContext *);
void stack_trace(MContext *);
void local_vars(MContext *);
void debug_help(MContext *);

#endif
