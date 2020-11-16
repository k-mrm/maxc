#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "object/mexception.h"
#include "vm.h"
#include "error.h"

NEW_EXCEPTION(exc_outofrange, "out_of_range");
NEW_EXCEPTION(exc_zero_division, "zero_division");
NEW_EXCEPTION(exc_assert, "assertion");
NEW_EXCEPTION(exc_file, "file");
NEW_EXCEPTION(exc_eof, "EOF");
NEW_EXCEPTION(exc_notfound, "not_found");

void mxc_raise(MException *e, char *msg, ...) {
  char buf[1024] = {0};
  int msg_size;
  MContext *c = curvm()->ctx;
  va_list arg;
  va_start(arg, msg);

  if((msg_size = vsnprintf(buf, sizeof(buf), msg, arg)) < 0) return;

  e->msg = (MString *)V2O(new_string_copy(buf, msg_size));
  c->exc = e;

  if(!c->err_handling_enabled)
    exc_report(e);
}

void exc_report(MException *e) {
  assert(e);
  MContext *c = curvm()->ctx;
  int lineno = curlineno(c->d, c->pc, c->basepc);
  fprintf(stderr, "%s:%d [%s error] %s\n",
      c->d->filename, lineno, e->errname, e->msg? e->msg->str : "");
  putsline(lineno);
  vm_force_exit(1);
}
