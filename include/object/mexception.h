#ifndef MXC_EXCEPTION_H
#define MXC_EXCEPTION_H

#include "object/object.h"
#include "object/mstr.h"

typedef struct MException MException;
struct MException {
  OBJECT_HEAD;
  char *errname;
  MString *msg;
};

#define EXC_OUTOFRANGE    (&exc_outofrange)
#define EXC_ZERO_DIVISION (&exc_zero_division)
#define EXC_ASSERT        (&exc_assert)
#define EXC_FILE          (&exc_file)
#define EXC_EOF           (&exc_eof)
#define EXC_NOTFOUND      (&exc_notfound)

#define NEW_EXCEPTION(exc, name)   \
  MException exc = {   \
    { NULL, GCGUARD_FLAG },    \
    name,   \
    NULL,   \
  }

extern MException exc_outofrange;
extern MException exc_zero_division;
extern MException exc_assert;
extern MException exc_file;
extern MException exc_eof;
extern MException exc_notfound;

void mxc_raise(MException *, char *, ...);
void exc_report(MException *);

#endif
