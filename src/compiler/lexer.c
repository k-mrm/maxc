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

char *strndup(const char *, size_t);

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

static void scan(Vector *, char *, const char *);

Vector *lexer_run(char *src, const char *fname) {
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
  ['\"'] = '\"',
  ['e'] = '\033',
  ['E'] = '\033'
};

char *escapedstrdup(char *s, size_t n) {
  char buf[n+1];
  int len = 0;
  char c;
  for(size_t i = 0; i < n; i++, s++) {
    if(*s == '\\') {
      i++;
      c = escaped[(int)*++s];
    }
    else {
      c = *s;
    }
    buf[len++] = c;
  }
  char *new = malloc(sizeof(char) * len);
  memset(new, 0, sizeof(char) * len);
  strncpy(new, buf, sizeof(char) * len);

  return new;
}

static void scan(Vector *tk, char *src, const char *fname) {
  int line = 1;
  int col = 1;
  size_t src_len = strlen(src);

  for(size_t i = 0; i < src_len; i++, col++) {
    if(isdigit(src[i])) {
      SrcPos start = cur_srcpos(fname, line, col);
      char *buf = src + i;
      bool isdot = false;

      int len = 0;
      for(; isdigit(src[i]) || (src[i] == '.' && src[i+1] != '.'); i++, col++) {
        len++;

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
        len--;
      }
      SrcPos end = cur_srcpos(fname, line, col);
      char *str = strndup(buf, len);
      token_push_num(tk, str, len, start, end);
    }
    else if(isalpha(src[i]) || src[i] == '_') {
      /* (alpha|_) (alpha|digit|_)* */
      char *ident_s = src + i;
      SrcPos start = cur_srcpos(fname, line, col);
      int len = 0;
      for(; isalpha(src[i]) || isdigit(src[i]) || src[i] == '_'; i++, col++) {
        len++;
      }

      PREV();
      char *ident = strndup(ident_s, len);
      SrcPos end = cur_srcpos(fname, line, col);
      token_push_ident(tk, ident, len, start, end);
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
      for(; src[i] != '\n' && src[i] != '\0'; i++, col++);

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
    else if(src[i] == '\"' || src[i] == '\'') {
      char q = src[i];
      SrcPos s = cur_srcpos(fname, line, col);
      STEP();
      char *buf = src + i;
      int len = 0;
      for(; src[i] != q; i++, col++) {
        if(src[i] == '\n') {
          error("missing character:`\"`");
          break;
        }
        len++;
      }
      SrcPos e = cur_srcpos(fname, line, col);

      char *str = escapedstrdup(buf, len);
      token_push_string(tk, str, len, s, e);
    }
    else if(src[i] == '`') {
      SrcPos s = cur_srcpos(fname, line, col);
      STEP();
      char *buf = src + i;

      int len = 0;
      for(; src[i] != '`'; i++, col++) {
        len++;

        if(src[i] == '\0') {
          error("missing charcter:'`'");
          return;
        }
      }

      char *str = strndup(buf, len);
      SrcPos e = cur_srcpos(fname, line, col);

      token_push_backquote_lit(tk, str, len, s, e);
    }
    else if(isblank(src[i])) {
      continue;
    }
    else if(src[i] == '\n') {
      line++;
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
