#ifndef MXC_RUNTIME_ERR_H
#define MXC_RUNTIME_ERR_H

#include "error/errortype.h"

struct Frame;

void mxc_raise_error(struct Frame *frame, enum RuntimeErrType);

#endif
