#include <string.h>
#include <stdarg.h>
#include "object/mexception.h"
#include "vm.h"

void mxc_raise(MException *e, char *msg, ...) {
  va_list arg;
  char buf[1024] = {0};
  int msg_size;
  MContext *c = curvm()->ctx;
  va_start(arg, msg);

  if((msg_size = vsprintf(buf, msg, arg)) < 0) return;


  e->msg = V2O(new_string_copy(buf, msg_size));
  c->exc = e;
}

void exc_report(MException *e) {
  ;
}
