#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "object/mexception.h"
#include "vm.h"

MException exc_outofrange = {
  { NULL, 0, 1, },
  "out_of_range",
  NULL,
};

MException exc_zero_division = {
  { NULL, 0, 1, },
  "zero_division",
  NULL,
};

MException exc_assert = {
  { NULL, 0, 1, },
  "assertion",
  NULL,
};

void mxc_raise(MException *e, char *msg, ...) {
  char buf[1024] = {0};
  int msg_size;
  MContext *c = curvm()->ctx;
  va_list arg;
  va_start(arg, msg);

  if((msg_size = vsprintf(buf, msg, arg)) < 0) return;

  e->msg = (MxcString *)V2O(new_string_copy(buf, msg_size));
  c->exc = e;
}

void exc_report(MException *e) {
  assert(e);
  fprintf(stderr, "[%s error] %s\n", e->errname, e->msg? e->msg->str : "");
}
