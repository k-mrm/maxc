#ifndef MAXC_H
#define MAXC_H

#include <stdbool.h>
#include "internal.h"
#include "util.h"

#ifdef __code_model_32__
typedef int32_t smptr_t;
typedef uint32_t mptr_t;
#else
typedef int64_t smptr_t;
typedef uint64_t mptr_t;
#endif

#if __GNUC__ >= 3
#define LIKELY(x)   (__builtin_expect(!!(x), 1))
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

typedef struct MInterp MInterp;
struct MInterp {
  int argc;
  char **argv;
  Vector *module;
  int errcnt;
};

int mxc_main_file(MInterp *, const char *);
int mxc_main_repl(MInterp *);

MInterp *mxc_open(int, char **);
void mxc_close(MInterp *);

#endif
