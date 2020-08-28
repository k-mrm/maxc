#ifndef MXC_EXCEPTION_H
#define MXC_EXCEPTION_H

#include "object/object.h"
#include "object/mstr.h"

enum exception {
  EOUTOFRANGE,
  EZERO_DIVISION,
  EASSERT,
};

typedef struct MException MException;
struct MException {
  OBJECT_HEAD;
  enum exception e;
  MxcString *msg;
};

#define EXC_OUTOFRANGE    (&exc_outofrange)
#define EXC_ZERO_DIVISION (&exc_zero_division)
#define EXC_ASSERT        (&exc_assert)

extern MException exc_outofrange;
extern MException exc_zero_division;
extern MException exc_assert;

void mxc_raise(MException *, char *, ...);
void exc_report(MException *);

#endif
