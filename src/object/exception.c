#include <string.h>
#include <stdarg.h>
#include "object/mexception.h"
#include "vm.h"

MException exc_outofrange = {
  { NULL, 0, 1, },
  EOUTOFRANGE,
  NULL,
};

MException exc_zero_division = {
  { NULL, 0, 1, },
  EZERO_DIVISION,
  NULL,
};

MException exc_assert = {
  { NULL, 0, 1, },
  EASSERT,
  NULL,
};

void mxc_raise(MException *e, char *msg, ...) {
  va_list arg;
  char buf[1024] = {0};
  int msg_size;
  MContext *c = curvm()->ctx;
  va_start(arg, msg);

  if((msg_size = vsprintf(buf, msg, arg)) < 0) return;

  e->msg = (MxcString *)V2O(new_string_copy(buf, msg_size));
  c->exc = e;
}

void exc_report(MException *e) {
  char *msg_head;
  switch(e->e) {
    case EOUTOFRANGE:     msg_head = "[out-of-range error]"; break;
    case EZERO_DIVISION:  msg_head = "[zero division error]"; break;
    case EASSERT:         msg_head = "[assertion failed]"; break;
    default:              msg_head = "?"; break;
  }

  fprintf(stderr, "%s %s\n", msg_head, e->msg? e->msg->str : "");
}
