#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "error/error.h"
#include "maxc.h"
#include "util.h"
#include "token.h"

#define STEP()                                                             \
  do {                                                                     \
    ++i;                                                                   \
    ++col;                                                                 \
  } while(0)
#define PREV()                                                             \
  do {                                                                     \
    --i;                                                                   \
    --col;                                                                 \
  } while(0)

#define TWOCHARS(c1, c2) (src[i] == c1 && src[i + 1] == c2)

static void scan(Vector *, const char *, const char *);

Vector *lexer_run(const char *src, const char *fname) {
  Vector *tokens = new_vector();
  scan(tokens, src, fname);

  return tokens;
}

static char escaped[256] = {
  ['a'] = '\a',
  ['b'] = '\b',
  ['f'] = '\f',
  ['n'] = '\n',
  ['r'] = '\r',
  ['t'] = '\t',
  ['v'] = '\v',
  ['\\'] = '\\',
  ['\''] = '\'',
  ['e'] = '\033',
  ['E'] = '\033'
};

static char scan_char(const char *src, size_t *idx, int *col) {
  if(src[*idx] != '\\') {
    return src[*idx];
  }
  ++(*idx); ++(*col);

  char res = escaped[(int)src[*idx]];
  if(res) return res;
  else {
    --(*idx); --(*col);
    return '\\';
  }
}

static void scan(Vector *tk, const char *src, const char *fname) {
  int line = 1;
  int col = 1;
  size_t src_len = strlen(src);

  for(size_t i = 0; i < src_len; ++i, ++col) {
    if(isdigit(src[i])) {
      SrcPos start = cur_srcpos(fname, line, col);
      String *value_num = New_String();
      bool isdot = false;

      for(; isdigit(src[i]) || (src[i] == '.' && src[i+1] != '.'); ++i, ++col) {
        string_push(value_num, src[i]);

        if(src[i] == '.') {
          if(isdot) break;
          isdot = true;
        }
      }
      PREV();
      if(src[i] == '.') {
        /*
         *  30.fibo()
         *    ^
         */
        PREV();
        string_pop(value_num);
      }
      SrcPos end = cur_srcpos(fname, line, col);
      token_push_num(tk, value_num, start, end);
    }
    else if(isalpha(src[i]) || src[i] == '_') {
      /* (alpha|_) (alpha|_)* */
      SrcPos start = cur_srcpos(fname, line, col);
      String *ident = New_String();

      for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_';
          ++i, ++col) {
        string_push(ident, src[i]);
      }

      PREV();
      SrcPos end = cur_srcpos(fname, line, col);
      token_push_ident(tk, ident, start, end);
    }
    else if(TWOCHARS('&', '&') || TWOCHARS('|', '|') ||
        TWOCHARS('.', '.') || TWOCHARS('>', '>') ||
        TWOCHARS('=', '>') || TWOCHARS('<', '<') ||
        TWOCHARS('-', '>')) {
      SrcPos s = cur_srcpos(fname, line, col);

      enum tkind kind = tk_char2(src[i], src[i + 1]);
      STEP();

      SrcPos e = cur_srcpos(fname, line, col);
      token_push_symbol(tk, kind, 2, s, e);
    }
    else if((src[i] == '/') && (src[i + 1] == '/')) {
      for(; src[i] != '\n' && src[i] != '\0'; ++i, ++col);

      PREV();
      continue;
    }
    else if(strchr("(){}&|[]:.,?;@#", src[i])) {
      SrcPos loc = cur_srcpos(fname, line, col);

      enum tkind kind = tk_char1(src[i]);
      token_push_symbol(tk, kind, 1, loc, loc);
    }
    else if(strchr("=<>!+-*/%", src[i])) {
      SrcPos s = cur_srcpos(fname, line, col);
      SrcPos e;

      enum tkind kind;
      if(src[i + 1] == '=') {
        kind = tk_char2(src[i], src[i + 1]);
        STEP();
        e = cur_srcpos(fname, line, col);
        token_push_symbol(tk, kind, 2, s, e);
      }
      else {
        kind = tk_char1(src[i]);
        e = cur_srcpos(fname, line, col);
        token_push_symbol(tk, kind, 1, s, e);
      }
    }
    else if(src[i] == '\"') {
      SrcPos s = cur_srcpos(fname, line, col);
      String *cont = New_String();
      STEP();
      for(; src[i] != '\"'; ++i, ++col) {
        if(src[i] == '\n') {
          error("missing character:`\"`");
          exit(1);
        }
        string_push(cont, scan_char(src, &i, &col));
      }
      SrcPos e = cur_srcpos(fname, line, col);

      token_push_string(tk, cont, s, e);
    }
    else if(src[i] == '\'') {
      SrcPos s = cur_srcpos(fname, line, col);
      STEP();
      if(src[i] == '\n') {
        error("missing character:`\'`");
        exit(1);
      }
      char res = scan_char(src, &i, &col);
      STEP();
      if(src[i] != '\'') {
        error("too long character");
        exit(1);
      }
      SrcPos e = cur_srcpos(fname, line, col);

      token_push_char(tk, res, s, e);
    }
    else if(src[i] == '`') {
      SrcPos s = cur_srcpos(fname, line, col);
      String *cont = New_String();
      STEP();

      for(; src[i] != '`'; ++i, ++col) {
        string_push(cont, src[i]);

        if(src[i] == '\0') {
          error("missing charcter:'`'");
          return;
        }
      }

      SrcPos e = cur_srcpos(fname, line, col);

      token_push_backquote_lit(tk, cont, s, e);
    }
    else if(isblank(src[i])) {
      continue;
    }
    else if(src[i] == '\n') {
      ++line;
      col = 0;
      continue;
    }
    else {
      error("invalid syntax: \" %c \"", src[i]);
      break;
    }
  }

  SrcPos eof = cur_srcpos(fname, ++line, col);
  token_push_end(tk, eof, eof);
}
