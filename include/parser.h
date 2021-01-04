#ifndef MAXC_PARSER_H
#define MAXC_PARSER_H

#include "util.h"

/* parser state */
struct mparser {
  struct mparser *prev;
  Vector *tokens;
  Vector *ast;
  int pos;
  int err;
};

struct mparser *parser_run(Vector *);

#endif
