#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "internal.h"
#include "ast.h"
#include "error/error.h"
#include "lexer.h"
#include "object/minteger.h"

struct mparser {
  struct mparser *prev;
  Vector *tokens;
  int pos;
};

static Ast *make_block(struct mparser *);
static Ast *statement(struct mparser *);
static Ast *expr(struct mparser *);
static Ast *expr_assign(struct mparser *);
static Ast *expr_equality(struct mparser *);
static Ast *expr_logic_or(struct mparser *);
static Ast *expr_bin_xor(struct mparser *);
static Ast *expr_logic_and(struct mparser *);
static Ast *expr_cmp(struct mparser *);
static Ast *expr_bitshift(struct mparser *);
static Ast *expr_add(struct mparser *);
static Ast *expr_mul(struct mparser *);
static Ast *expr_unary(struct mparser *);
static Ast *expr_unary_postfix(struct mparser *);
static Ast *expr_unary_postfix_atmark(struct mparser *);
static Ast *expr_primary(struct mparser *);
static Ast *new_object(struct mparser *, int);
static Ast *make_list_with_size(struct mparser *, Ast *, int);
static Ast *var_decl(struct mparser *, bool, int);
static Ast *expr_num(Token *);
static Ast *expr_unary(struct mparser *);
static Type *eval_type(struct mparser *);
static Token *see(struct mparser *, int);
static Vector *enter(Vector *, struct mparser *p);

#define step(p) (p->pos++)
#define curtk(p) ((Token *)p->tokens->data[p->pos])
#define fetchtk(p) ((Token *)p->tokens->data[p->pos++])
#define curtk_is(p, tk) ((curtk(p)->kind) == (tk))

#define curline(p) (curtk(p)->start.line)

static struct mparser *new_mparser(Vector *ts, struct mparser *prev) {
  struct mparser *p = malloc(sizeof(struct mparser));
  p->prev = prev;
  p->tokens = ts;
  p->pos = 0;
  return p;
}

static void delete_mparser(struct mparser *p) {
  del_vector(p->tokens);
  free(p);
}

static Token *skip(struct mparser *p, enum tkind tk) {
  Token *cur = curtk(p);
  if(cur->kind == tk) {
    p->pos++;
    return cur;
  }
  return NULL;
}

static Token *see(struct mparser *p, int off) { return p->tokens->data[p->pos + off]; }

static bool skip2(struct mparser *p, enum tkind tk1, enum tkind tk2) {
  int tmp = p->pos;
  if(curtk(p)->kind == tk1) {
    p->pos++;
    if(curtk(p)->kind == tk2) {
      p->pos++;
      return true;
    }
  }

  p->pos = tmp;
  return false;
}

static Token *expect(struct mparser *p, enum tkind tk) {
  Token *cur = curtk(p);
  if(cur->kind == tk) {
    p->pos++;
    return cur;
  }
  else {
    unexpected_token(cur, tkind2str(tk), NULL);
    return NULL;
  }
}

static char *eat_identifer(struct mparser *p) {
  Token *tk = fetchtk(p);
  if(tk->kind != TKIND_Identifer) {
    unexpected_token(tk, "Identifer", NULL);
    return NULL;
  }
  return tk->value;
}

static void skip_to(struct mparser *p, enum tkind tk) {
  while(!curtk_is(p, tk) && !curtk_is(p, TKIND_End)) {
    step(p);
  }
}

static bool is_expr_tk(struct mparser *p) {
  switch(curtk(p)->kind) {
    case TKIND_Num: case TKIND_String:
    case TKIND_Identifer: case TKIND_Lboxbracket:
    case TKIND_Bang: case TKIND_True:
    case TKIND_False: case TKIND_New:
      return true;
    default: return false;
  }
}

static Ast *expr(struct mparser *p) { return expr_assign(p); }

static Ast *func_def(struct mparser *p, bool iter, int line) {
  bool is_operator = false;
  bool is_generic = false;
  int op = -1;

  Vector *typevars = NULL;

  if(curtk(p)->kind == TKIND_BQLIT) {
    is_operator = true;
    op = curtk(p)->cont;

    if(op == -1) {
      error("operators that cannot be overloaded");
    }
  }

  int fname_line = curline(p);
  char *name = eat_identifer(p);

  // def main(): typename {
  //         ^
  if(!expect(p, TKIND_Lparen)) {
    return NULL;
  }

  Vector *args = new_vector();
  Vector *argtys = new_vector();

  /*
   * def main(a: int, b: int): int {
   *          ^^^^^^^^^^^^^^
   *
   * def main(a, b: int): int {
   *          ^^^^^^^^^
   */
  for(int i = 0; !skip(p, TKIND_Rparen); i++) {
    if(i > 0) {
      expect(p, TKIND_Comma);
    }
    Vector *argnames = new_vector();
    Vector *posbuf = new_vector();

    do {
      int line = curline(p);
      char *name = eat_identifer(p);
      if(!name) {
        skip_to(p, TKIND_Rparen);
        return NULL;
      }
      vec_push(argnames, name);
      vec_push(posbuf, (void *)(intptr_t)line);
      if(curtk_is(p, TKIND_Colon)) break;

      expect(p, TKIND_Comma);
    } while(1);

    expect(p, TKIND_Colon);

    Type *cur_ty = eval_type(p);

    for(int i = 0; i < argnames->len; i++)
      vec_push(argtys, cur_ty);

    for(int i = 0; i < argnames->len; i++) {
      vec_push(args,
          node_variable_type(argnames->data[i], 0, cur_ty, (int)(intptr_t)posbuf->data[i]));
    }

    del_vector(argnames);
    del_vector(posbuf);
  }

  /*
   *  def main(): int {
   *            ^^^^^
   *  def main() = expr;
   */
  Type *ret_ty = skip(p, TKIND_Colon) ? eval_type(p) : NULL;
  Type *fntype = iter? new_type_generator(argtys, ret_ty) : new_type_function(argtys, ret_ty);
  Ast *block;

  if(curtk_is(p, TKIND_Lbrace)) { 
    block = make_block(p);
    if(ret_ty == NULL)
      fntype->fnret = mxc_none;
  }
  else if(curtk_is(p, TKIND_Assign)) {
    step(p);
    block = expr(p);
    expect(p, TKIND_Semicolon);
    if(ret_ty == NULL)
      fntype->fnret = new_type(CTYPE_UNINFERRED);
  }
  else {
    unexpected_token(curtk(p), "=", "{", NULL);
    block = NULL;
  }
  NodeVariable *function = node_variable_type(name, 0, fntype, fname_line);
  NodeFunction *node = node_function(function, block, typevars, args, iter, line);

  if(is_operator)
    node->op = op;

  return (Ast *)node;
}

static Ast *var_decl_block(struct mparser *p, bool isconst, int line) {
  Vector *block = new_vector();

  for(;;) {
    Type *ty = NULL;
    Ast *init = NULL;
    NodeVariable *var = NULL;

    int vnameline = curline(p);
    char *name = eat_identifer(p);
    if(!name) {
      skip_to(p, TKIND_Rbrace);
      return NULL;
    }

    if(skip(p, TKIND_Colon))
      ty = eval_type(p);
    else
      ty = new_type(CTYPE_UNINFERRED);
    int vattr = 0;

    if(isconst) vattr |= VARATTR_CONST;

    if(skip(p, TKIND_Assign))
      init = expr(p);
    else if(isconst)
      error_at(see(p, 0)->start, see(p, 0)->end, "const must initialize");

    var = node_variable_type(name, vattr, ty, vnameline);
    expect(p, TKIND_Semicolon);
    vec_push(block, node_vardecl(var, init, NULL, vnameline));

    if(skip(p, TKIND_Rbrace)) break;
  }

  return (Ast *)node_vardecl(NULL, NULL, block, line);
}

static Ast *var_decl(struct mparser *p, bool isconst, int line) {
  if(skip(p, TKIND_Lbrace)) {
    return var_decl_block(p, isconst, line);
  }

  int vattr = 0;
  if(isconst) vattr |= VARATTR_CONST;

  Ast *init = NULL;
  Type *ty = NULL;
  NodeVariable *var = NULL;
  int vnameline = curline(p);

  char *name = eat_identifer(p);
  if(!name) {
    skip_to(p, TKIND_Semicolon);
    return NULL;
  }
  /*
   *  let a(: int) = 100;
   *        ^^^^^
   */
  ty = skip(p, TKIND_Colon) ? eval_type(p) : new_type(CTYPE_UNINFERRED);
  /*
   *  let a: int = 100;
   *             ^
   */
  if(skip(p, TKIND_Assign)) {
    init = expr(p);
    if(!init) return NULL;
  }
  else if(isconst) {
    error_at(see(p, 0)->start, see(p, 0)->end, "const must initialize");
    init = NULL;
  }
  else {
    init = NULL;
  }
  var = node_variable_type(name, vattr, ty, vnameline);
  expect(p, TKIND_Semicolon);

  return (Ast *)node_vardecl(var, init, NULL, line);
}

static Ast *make_object(struct mparser *p, int line) {
  /*
   *  object TagName {
   *      a: int,
   *      b: string
   *  }
   */
  char *tag = fetchtk(p)->value;

  expect(p, TKIND_Lbrace);

  Vector *decls = new_vector();

  for(int i = 0; !skip(p, TKIND_Rbrace); i++) {
    if(i > 0)
      expect(p, TKIND_Comma);
    int line = curline(p);
    char *name = eat_identifer(p);
    if(!name) {
      skip_to(p, TKIND_Rbrace);
      return NULL;
    }

    expect(p, TKIND_Colon);

    Type *ty = eval_type(p);
    vec_push(decls, node_variable_type(name, 0, ty, line));
  }

  return (Ast *)node_object(tag, decls, line);
}

static int make_ast_from_mod(struct mparser *p, Vector *s, char *name) {
  char path[512];
  sprintf(path, "./lib/%s.mxc", name);
  char *src = read_file(path);
  if(!src) {
    memset(path, 0, 512);
    sprintf(path, "./%s.mxc", name);

    src = read_file(path);
    if(!src) {
      error_at(see(p, -1)->start, see(p, -1)->end, "lib %s: not found", name);
      return 1;
    }
  }

  Vector *token = lexer_run(src, name);
  Vector *ast = enter(token, p);
  for(int i = 0; i < ast->len; i++) {
    vec_push(s, ast->data[i]);
  }

  return 0;
}

static Ast *make_moduse(struct mparser *p, int line) {
  Vector *mod_names = new_vector();
  Vector *statements = new_vector();
  char *mod = fetchtk(p)->value;

  if(make_ast_from_mod(p, statements, mod))
    return NULL;

  NodeBlock *block = node_block(statements, line);
  expect(p, TKIND_Semicolon);

  return (Ast *)node_namespace(mod, block, line);
}

static Ast *make_breakpoint(struct mparser *p, int line) {
  Ast *a = (Ast *)node_breakpoint(line);
  expect(p, TKIND_Semicolon);

  return a;
}

static Ast *make_assert(struct mparser *p, int line) {
  Ast *a = expr(p);
  NodeAssert *as = node_assert(a, line);
  expect(p, TKIND_Semicolon);

  return (Ast *)as;
}

static Type *eval_type(struct mparser *p) {
  Type *ty;

  if(skip(p, TKIND_Lparen)) { // tuple
    ty = new_type(CTYPE_TUPLE);

    for(;;) {
      vec_push(ty->tuple, eval_type(p));

      if(skip(p, TKIND_Rparen))
        break;

      expect(p, TKIND_Comma);
    }
  }
  else if(skip(p, TKIND_TInt))
    ty = mxc_int;
  else if(skip(p, TKIND_TUint))
    ty = new_type(CTYPE_UINT);
  else if(skip(p, TKIND_TBool))
    ty = mxc_bool;
  else if(skip(p, TKIND_TString))
    ty = mxc_string;
  else if(skip(p, TKIND_TFloat))
    ty = mxc_float;
  else if(skip(p, TKIND_TFile))
    ty = mxc_file;
  else if(skip(p, TKIND_TNone)) // TODO :only function rettype
    ty = mxc_none;
  else if(skip(p, TKIND_Def)) {
    expect(p, TKIND_Lparen);
    Vector *arg = new_vector();

    while(!skip(p, TKIND_Rparen)) {
      vec_push(arg, eval_type(p));
      if(skip(p, TKIND_Rparen))
        break;
      expect(p, TKIND_Comma);
    }
    expect(p, TKIND_Colon);
    Type *ret = eval_type(p);
    ty = new_type_function(arg, ret);
  }
  else {
    char *tk = curtk(p)->value;
    step(p);
    ty = new_type_unsolved(tk);
  }

  for(;;) {
    if(skip2(p, TKIND_Lboxbracket, TKIND_Rboxbracket))
      ty = new_type_list(ty);
    else
      break;
  }

  return ty;
}

Ast *make_assign(Ast *dst, Ast *src, int line) {
  if(!dst) return NULL;
  return (Ast *)node_assign(dst, src, line);
}

static Ast *make_block(struct mparser *p) {
  int line = curline(p);
  expect(p, TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;
  for(;;) {
    if(skip(p, TKIND_Rbrace))
      break;
    b = statement(p);

    if(ast_isexpr(b))
      expect(p, TKIND_Semicolon);
    vec_push(cont, b);
  }

  return (Ast *)node_block(cont, line);
}

static Ast *make_typed_block(struct mparser *p) {
  int line = curline(p);
  expect(p, TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;

  for(;;) {
    if(skip(p, TKIND_Rbrace))
      break;
    b = statement(p);

    if(ast_isexpr(b))
      expect(p, TKIND_Semicolon);
    vec_push(cont, b);
  }

  return (Ast *)node_typedblock(cont, line);
}

static Ast *make_if(struct mparser *p, bool isexpr, int line) {
  Ast *cond = expr(p);
  Ast *then = isexpr ? expr(p) : make_block(p);
  Token *if_c;

  if(skip(p, TKIND_Else)) {
    Ast *el;

    if((if_c = skip(p, TKIND_If)))
      el = make_if(p, isexpr, if_c->start.line);
    else
      el = isexpr? expr(p) : make_block(p);

    return (Ast *)node_if(cond, then, el, isexpr, line);
  }
  return (Ast *)node_if(cond, then, NULL, isexpr, line);
}

static Ast *make_for(struct mparser *p, int line) {
  /*
   *  for i in iter() { }
   */

  Vector *v = new_vector();

  do {
    int loopvline = curline(p);
    char *loopv = eat_identifer(p);
    Type *ty = new_type(CTYPE_UNINFERRED); 
    vec_push(v, node_variable_type(loopv, 0, ty, loopvline));
  } while(!skip(p, TKIND_In));

  Ast *iter = expr(p);
  if(!iter) return NULL;

  Ast *body = (Ast *)make_block(p);
  if(!body) return NULL;

  return (Ast *)node_for(v, iter, body, line);
}

static Ast *make_while(struct mparser *p, int line) {
  Ast *cond = expr(p);
  Ast *body = make_block(p);

  return (Ast *)node_while(cond, body, line);
}

/*
 *  switch n {
 *    case 10: echo 10;
 *    case 20: echo 20;
 *    case 30: echo 30;
 *    else: echo "?";
 *  }
 *
 */

static Ast *make_switch(struct mparser *p, int line) {
  Ast *match = expr(p);
  expect(p, TKIND_Lbrace);
  Vector *ecase = new_vector();
  Vector *body = new_vector();
  Ast *eelse = NULL;
  int felse = 0;

  do {
    if(skip(p, TKIND_CASE)) {
      vec_push(ecase, expr(p));
      expect(p, TKIND_Colon);
      vec_push(body, statement(p));
      expect(p, TKIND_Semicolon);
    }
    else if(skip(p, TKIND_Else) && !felse) {
      expect(p, TKIND_Colon);
      eelse = statement(p);
      expect(p, TKIND_Semicolon);
      felse = 1;
    }
    else {
      /* error */
    }
  } while(!skip(p, TKIND_Rbrace));

  return (Ast *)node_switch(match, ecase, body, eelse, line);
}

static Ast *make_return(struct mparser *p, int line) {
  Ast *e = expr(p);
  NodeReturn *ret = node_return(e, line);
  if(e->type != NDTYPE_NONENODE)
    expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_yield(struct mparser *p, int line) {
  Ast *e = expr(p);
  NodeYield *ret = node_yield(e, line);
  if(e->type != NDTYPE_NONENODE)
    expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_break(struct mparser *p, int line) {
  NodeBreak *ret = node_break(line);
  expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_skip(struct mparser *p, int line) {
  NodeSkip *ret = node_skip(line);
  expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static void make_typedef(struct mparser *p) {
  mxc_unimplemented("typedef");
}

static Ast *expr_num(Token *tk) {
  int line = tk->start.line;
  if(strchr(tk->value, '.'))
    return (Ast *)node_number_float(atof(tk->value), line);

  int overflow = 0;
  size_t len;
  int32_t i = intern_scan_digiti32(tk->value, 10, &overflow, &len);
  if(overflow) {
    MxcValue a = new_integer(tk->value, 10);
    return (Ast *)node_number_big(a, line);
  }
  return (Ast *)node_number_int(i, line);
}

static Ast *expr_string(Token *tk) { return (Ast *)node_string(tk->value, tk->start.line); }

static Ast *expr_var(struct mparser *p) {
  int l = curline(p);
  char *name = eat_identifer(p);
  if(!name) return NULL;

  return (Ast *)node_variable(name, 0, l);
}

static Ast *expr_catcherr(struct mparser *p);

static Ast *expr_assign(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_catcherr(p);

  if(curtk_is(p, TKIND_Assign)) {
    step(p);
    left = make_assign(left, expr_assign(p), curl);
  }
  else if(curtk_is(p, TKIND_PlusAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_ADD, left, expr_assign(p), curl), curl);
  }
  else if(curtk_is(p, TKIND_MinusAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_SUB, left, expr_assign(p), curl), curl);
  }
  else if(curtk_is(p, TKIND_AsteriskAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_MUL, left, expr_assign(p), curl), curl);
  }
  else if(curtk_is(p, TKIND_DivAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_DIV, left, expr_assign(p), curl), curl);
  }
  else if(curtk_is(p, TKIND_ModAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_MOD, left, expr_assign(p), curl), curl);
  }

  return left;
}

static Ast *expr_range(struct mparser *);

static Ast *expr_catcherr(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_range(p);
  Ast *r;

  for(;;) {
    if(curtk_is(p, TKIND_Question)) {
      step(p);
      r = expr_range(p);
      left = (Ast *)node_binary(BIN_QUESTION, left, r, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_range(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_logic_or(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_DotDot)) {
      step(p);
      t = expr_logic_or(p);
      left = (Ast *)node_binary(BIN_DOTDOT, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_or(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_logic_and(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_LogOr) || curtk_is(p, TKIND_KOr)) {
      step(p);
      t = expr_logic_and(p);
      left = (Ast *)node_binary(BIN_LOR, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_and(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_bin_xor(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_LogAnd) || curtk_is(p, TKIND_KAnd)) {
      step(p);
      t = expr_bin_xor(p);
      left = (Ast *)node_binary(BIN_LAND, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_bin_xor(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_equality(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Xor)) {
      step(p);
      t = expr_equality(p);
      left = (Ast *)node_binary(BIN_BXOR, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_equality(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_cmp(p);

  for(;;) {
    if(curtk_is(p, TKIND_Eq)) {
      step(p);
      Ast *t = expr_cmp(p);
      left = (Ast *)node_binary(BIN_EQ, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Neq)) {
      step(p);
      Ast *t = expr_cmp(p);
      left = (Ast *)node_binary(BIN_NEQ, left, t, curl);
    }
    else
      return left;
  }
}

static Ast *expr_cmp(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_bitshift(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Lt)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_LT, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Gt)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_GT, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Lte)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_LTE, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Gte)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_GTE, left, t, curl);
    }
    else
      return left;
  }
}

static Ast *expr_bitshift(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_add(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Lshift)) {
      step(p);
      t = expr_add(p);
      left = (Ast *)node_binary(BIN_LSHIFT, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Rshift)) {
      step(p);
      t = expr_add(p);
      left = (Ast *)node_binary(BIN_RSHIFT, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_add(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_mul(p);

  for(;;) {
    if(curtk_is(p, TKIND_Plus)) {
      step(p);
      Ast *t = expr_mul(p);
      left = (Ast *)node_binary(BIN_ADD, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Minus)) {
      step(p);
      Ast *t = expr_mul(p);
      left = (Ast *)node_binary(BIN_SUB, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_mul(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_unary(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Asterisk)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_MUL, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Div)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_DIV, left, t, curl);
    }
    else if(curtk_is(p, TKIND_Mod)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_MOD, left, t, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_unary(struct mparser *p) {
  int curl = curline(p);
  enum tkind tk = curtk(p)->kind;
  enum UNAOP op = -1;

  switch(tk) {
    case TKIND_Minus: op = UNA_MINUS; break;
    case TKIND_Bang:  op = UNA_NOT; break;
    default: return expr_unary_postfix(p);
  }

  step(p);
  Ast *operand = expr_unary(p);
  if(operand == NONE_NODE) {
    error_at(see(p, -1)->start, see(p, -1)->end,
        "expected expression before `;`");
    return NULL;
  }

  return (Ast *)node_unary(op, operand, curl);
}

static Ast *expr_unary_postfix(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_unary_postfix_atmark(p);

  for(;;) {
    if(curtk_is(p, TKIND_Dot)) {
      step(p);

      Ast *memb = expr_unary_postfix_atmark(p);
      /*
       *  a.function();
       *            ^
       */
      if(skip(p, TKIND_Lparen)) {
        Vector *args = new_vector();
        vec_push(args, left);

        for(int i = 0; !skip(p, TKIND_Rparen); i++) {
          if(i > 0) {
            expect(p, TKIND_Comma);
          }

          vec_push(args, expr(p));
        }

        left = (Ast *)node_fncall(memb, args, curl);
      }
      else if(is_expr_tk(p)) {
        Vector *args = new_vector();
        vec_push(args, left);
        do {
          vec_push(args, expr(p));
        } while(skip(p, TKIND_Comma));
        left = (Ast *)node_fncall(memb, args, curl);
      }
      else { // struct
        left = (Ast *)node_dotexpr(left, memb, curl);
      }
    }
    else if(curtk_is(p, TKIND_Lboxbracket)) {
      step(p);
      Ast *index = expr(p);
      expect(p, TKIND_Rboxbracket);

      left = (Ast *)node_subscript(left, index, curl);
    }
    else if(curtk_is(p, TKIND_Lparen)) {
      step(p);
      Vector *args = new_vector();

      if(skip(p, TKIND_Rparen)) {}
      else {
        for(;;) {
          vec_push(args, expr(p));
          if(skip(p, TKIND_Rparen))
            break;
          expect(p, TKIND_Comma);
        }
      }
      left = (Ast *)node_fncall(left, args, curl);
    }
    else if(is_expr_tk(p)) {
      Vector *args = new_vector();

      do {
        vec_push(args, expr(p));
      } while(skip(p, TKIND_Comma));

      left = (Ast *)node_fncall(left, args, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_unary_postfix_atmark(struct mparser *p) {
  int curl = curline(p);
  Ast *left = expr_primary(p);

  for(;;) {
    if(curtk_is(p, TKIND_Atmark)) {
      step(p);
      Ast *ident = expr_var(p);

      left = (Ast *)node_modulefunccall(left, ident, curl);
    }
    else {
      return left;
    }
  }
}

static Ast *make_hash(struct mparser *p, int line) {
  /*
   *   let table = #["a":100, "b":200];
   *   let niltable = #;
   */
  if(skip(p, TKIND_Lboxbracket)) {
    Vector *k = new_vector();
    Vector *v = new_vector();

    for(int i = 0; !skip(p, TKIND_Rboxbracket); i++) {
      if(i > 0)
        expect(p, TKIND_Comma);

      vec_push(k, expr(p));
      expect(p, TKIND_Colon);
      vec_push(v, expr(p));
    }

    return (Ast *)node_hashtable(k, v, line);
  }
  else {
    return (Ast *)node_hashtable(NULL, NULL, line);
  }
}

static Ast *make_list_with_size(struct mparser *p, Ast *nelem, int line) {
  Ast *init = expr(p);
  expect(p, TKIND_Rboxbracket);

  return (Ast *)node_list(NULL, 0, nelem, init, line); 
}

static Ast *expr_primary(struct mparser *p) {
  Token *c;
  if((c = skip(p, TKIND_True))) {
    return (Ast *)node_bool(true, c->start.line);
  }
  else if((c = skip(p, TKIND_False))) {
    return (Ast *)node_bool(false, c->start.line);
  }
  else if((c = skip(p, TKIND_Null))) {
    return (Ast *)node_null(c->start.line);
  }
  else if((c = skip(p, TKIND_New))) {
    return new_object(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Hash))) {
    return make_hash(p, c->start.line);
  }
  else if(curtk_is(p, TKIND_Identifer)) {
    Ast *v = expr_var(p);
    return v;
  }
  else if(curtk_is(p, TKIND_Num)) {
    return expr_num(fetchtk(p));
  }
  else if(curtk_is(p, TKIND_String)) {
    return expr_string(fetchtk(p));
  }
  else if(skip(p, TKIND_Lparen)) {
    if(skip(p, TKIND_Rparen))
      return NULL;

    Ast *left = expr(p);

    if(!expect(p, TKIND_Rparen))
      step(p);

    return left;
  }
  else if((c = skip(p, TKIND_Lboxbracket))) {
    Vector *elem = new_vector();
    Ast *a;

    for(int i = 0; !skip(p, TKIND_Rboxbracket); i++) {
      if(i > 0) {
        if(skip(p, TKIND_Semicolon)) {
          del_vector(elem);
          return make_list_with_size(p, a, c->start.line);
        }
        expect(p, TKIND_Comma);
      }
      a = expr(p);
      vec_push(elem, a);
    }

    return (Ast *)node_list(elem, elem->len, NULL, NULL, c->start.line);
  }
  else if(skip(p, TKIND_Semicolon)) {
    return NONE_NODE;
  }
  else if(curtk_is(p, TKIND_End)) {
    return NULL;
  }
  error_at(see(p, 0)->start, see(p, 0)->end, "syntax error");
  skip_to(p, TKIND_Semicolon);

  return NULL;
}

static Ast *new_object(struct mparser *p, int line) {
  /*
   *  let a = new Data {
   *    member1: 100,
   *    member2: "hogehoge"
   *  };
   */
  char *tagname = fetchtk(p)->value;
  Type *tag = new_type_unsolved(tagname);
  Vector *fields = NULL,
         *inits = NULL;

  if(skip(p, TKIND_Lbrace)) {
    fields = new_vector();
    inits = new_vector();

    for(int i = 0; !skip(p, TKIND_Rbrace); i++) {
      if(i > 0)
        expect(p, TKIND_Comma);
      vec_push(fields, expr_var(p));
      expect(p, TKIND_Colon);

      vec_push(inits, expr(p));
    }
  }

  return (Ast *)node_struct_init(tag, fields, inits, line);
}

static Ast *statement(struct mparser *p) {
  Token *c;

  if(curtk_is(p, TKIND_Lbrace)) {
    return make_block(p);
  }
  else if((c = skip(p, TKIND_For))) {
    return make_for(p, c->start.line);
  }
  else if((c = skip(p, TKIND_While))) {
    return make_while(p, c->start.line);
  }
  else if((c = skip(p, TKIND_If))) {
    return make_if(p, false, c->start.line);
  }
  else if((c = skip(p, TKIND_Return))) {
    return make_return(p, c->start.line);
  }
  else if((c = skip(p, TKIND_YIELD))) {
    return make_yield(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Break))) {
    return make_break(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Skip))) {
    return make_skip(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Let))) {
    return var_decl(p, false, c->start.line);
  }
  else if((c = skip(p, TKIND_Const))) {
    return var_decl(p, true, c->start.line);
  }
  else if((c = skip(p, TKIND_ITERATOR))) {
    return func_def(p, true, c->start.line);
  }
  else if((c = skip(p, TKIND_Def))) {
    return func_def(p, false, c->start.line);
  }
  else if((c = skip(p, TKIND_Object))) {
    return make_object(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Use))) {
    return make_moduse(p, c->start.line);
  }
  else if((c = skip(p, TKIND_Assert))) {
    return make_assert(p, c->start.line);
  }
  else if((c = skip(p, TKIND_SWITCH))) {
    return make_switch(p, c->start.line);
  }
  else if(skip(p, TKIND_Typedef)) {
    make_typedef(p);
    return NULL;
  }
  else {
    return expr(p);
  }
}

static Vector *parser_main(struct mparser *p) {
  Vector *program = new_vector();

  while(!curtk_is(p, TKIND_End)) {
    Ast *st = statement(p);
    if(st)
      vec_push(program, st);

    if(ast_isexpr(st))
      expect(p, TKIND_Semicolon);
  }

  return program;
}

static Vector *enter(Vector *ts, struct mparser *prev) {
  struct mparser *p = new_mparser(ts, prev);
  Vector *result = parser_main(p);
  delete_mparser(p);
  return result;
}

Vector *parser_run(Vector *ts) {
  return enter(ts, NULL);
}

