#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "error/error.h"
#include "maxc.h"

extern char *filename;
extern char *code;
int errcnt = 0;

static void errheader(SrcPos start, SrcPos end) {
  fprintf(stderr,
      "\e[31;1m[error]\e[0m\e[1m(line %d:col %d): ",
      start.line,
      end.col);
}

void putsline(int lineno) {
  if(lineno < 0) {
    return;
  }
  log_error("\e[36;1m%d | \e[0m", lineno);

  int curline = 1;
  for(size_t i = 0; i < strlen(code); i++) {
    if(curline == lineno) {
      char *cur = code + i;
      int len = 0;
      while(code[i++] != '\n')
        len++;

      log_error("%.*s", len, cur);
      break;
    }

    if(code[i] == '\n')
      ++curline;
  }

  log_error("\n");
}

void error(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[31;1m[error] \e[0m\e[1m");
  vfprintf(stderr, msg, args);
  log_error("\e[0m\n");
  if(filename)
    log_error("\e[33;1min %s\e[0m\n", filename);
  va_end(args);

  // our_interp()->errcnt++;
}

void errline(int line, char *msg, ...) {
  log_error("\e[31;1m[error]\e[0m\e[1m");
  log_error("(line %d): ", line);
  log_error("\e[0m");

  va_list args;
  va_start(args, msg);
  log_error("\e[1m");
  vfprintf(stderr, msg, args);
  va_end(args);
  log_error("\e[0m");
  log_error(STR_DEFAULT "\n\n");

  putsline(line);

  log_error("\n");
}

void error_nofile(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[31;1m[error] \e[0m");
  log_error("\e[1m");
  vfprintf(stderr, msg, args);
  log_error("\e[0m\n");
  va_end(args);

  // our_interp()->errcnt++;
}

void warn(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[94;1m[warning] \e[0m");
  vfprintf(stderr, msg, args);
  log_error("\e[0m");
  if(filename)
    log_error("\e[33;1min %s\e[0m ", filename);
  log_error("\n");
  va_end(args);
}

void error_at(const SrcPos start, const SrcPos end, const char *msg, ...) {
  errheader(start, end);

  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  log_error(STR_DEFAULT "\n");

  int lline = end.line - start.line + 1;
  int lcol = end.col - start.col + 1;

  if(start.filename) {
    log_error("\e[33;1min %s\e[0m ", start.filename);
    log_error("\n\n");
  }

  putsline(start.line);

  log_error("%*s", start.col + get_digit(start.line) + 2, " ");
  log_error("\e[31;1m");
  log_error("%*s", lcol, "^");
  log_error(STR_DEFAULT);
  log_error("\n\n");
  va_end(args);

  // our_interp()->errcnt++;
}

void unexpected_token(Token *tk, ...) {
  SrcPos start = tk->start;
  SrcPos end = tk->end;
  errheader(start, end);

  char *unexpected = tk2str(tk);
  log_error("unexpected token: `%s`", unexpected);
  log_error(STR_DEFAULT "\n");

  int lline = end.line - start.line + 1;
  int lcol = end.col - start.col + 1;

  if(start.filename) {
    log_error("\e[33;1min %s\e[0m ", start.filename);
    log_error("\n\n");
  }

  putsline(start.line);

  for(size_t i = 0; i < start.col + get_digit(start.line) + 2; ++i)
    log_error(" ");
  log_error("\e[31;1m");
  for(int i = 0; i < lcol; ++i)
    log_error("^");
  log_error(" expected: ");

  va_list expect;
  va_start(expect, tk);

  int ite = 0;
  for(char *t = va_arg(expect, char *); t; t = va_arg(expect, char *), ite++) {
    if(ite > 0) {
      log_error(", ");
    }
    log_error("`%s`", t);
  }

  log_error(STR_DEFAULT "\n\n");

  // our_interp()->errcnt++;
}

void mxc_unimplemented(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[31;1m[unimplemented] \e[0m");
  if(filename)
    log_error("\e[1m%s:\e[0m ", filename);
  vfprintf(stderr, msg, args);
  log_error("\n");
  va_end(args);

  // our_interp()->errcnt++;
}

void warning(const SrcPos start, const SrcPos end, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[34;1m[warning]\e[0m\e[1m(line %d:col %d): ",
      start.line,
      start.col);
  vfprintf(stderr, msg, args);
  log_error(STR_DEFAULT);
  if(filename) {
    log_error("\e[33;1min %s\e[0m ", start.filename);
    log_error("\n\n");
  }
  va_end(args);
}

void debug(const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  log_error("\e[33;1m[debug] \e[0m");
  vfprintf(stderr, msg, args);
  va_end(args);
}

void mxc_assert_core(int boolean, char *message, char *file, int line) {
  if(boolean == false) {
    log_error("\e[31;1m[assertion failed]: \e[0m");
    log_error("\e[1m%s (%s:%d)\n\e[0m", message, file, line);
  }
}
