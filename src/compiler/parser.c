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
static Ast *expr_comp(struct mparser *);
static Ast *expr_bitshift(struct mparser *);
static Ast *expr_add(struct mparser *);
static Ast *expr_mul(struct mparser *);
static Ast *expr_unary(struct mparser *);
static Ast *expr_unary_postfix(struct mparser *);
static Ast *expr_unary_postfix_atmark(struct mparser *);
static Ast *expr_primary(struct mparser *);
static Ast *new_object(struct mparser *);
static Ast *make_list_with_size(struct mparser *, Ast *);
static Ast *var_decl(struct mparser *, bool);
static Ast *expr_char(struct mparser *);
static Ast *expr_num(Token *);
static Ast *expr_unary(struct mparser *);
static Type *eval_type(struct mparser *);
static Token *see(struct mparser *, int);
static Vector *enter(Vector *, struct mparser *p);

#define step(p) (++p->pos)
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

static bool skip(struct mparser *p, enum tkind tk) {
  if(curtk(p)->kind == tk) {
    ++p->pos;
    return true;
  }
  return false;
}

static Token *see(struct mparser *p, int off) { return p->tokens->data[p->pos + off]; }

static bool skip2(struct mparser *p, enum tkind tk1, enum tkind tk2) {
  int tmp = p->pos;
  if(curtk(p)->kind == tk1) {
    ++p->pos;
    if(curtk(p)->kind == tk2) {
      ++p->pos;
      return true;
    }
  }

  p->pos = tmp;
  return false;
}

static Token *expect(struct mparser *p, enum tkind tk) {
  Token *cur = curtk(p);
  if(cur->kind == tk) {
    ++p->pos;
    return cur;
  }
  else {
    expect_token(see(p, -1)->start, see(p, -1)->end, tk2str(tk));
    return NULL;
  }
}

static Token *expect_type(struct mparser *p, enum tkind tk) {
  Token *cur = curtk(p);
  if(cur->kind == tk) {
    ++p->pos;
    return cur;
  }
  else {
    expect_token(see(p, 0)->start, see(p, 0)->end, tk2str(tk));
    return NULL;
  }
}

static char *eat_identifer(struct mparser *p) {
  Token *tk = fetchtk(p);
  if(tk->kind != TKIND_Identifer) {
    unexpected_token(tk->start,
        tk->end,
        tk->value,
        "Identifer", NULL);
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
    case TKIND_Char: case TKIND_Identifer:
    case TKIND_Lboxbracket: case TKIND_Bang:
    case TKIND_True: case TKIND_False: case TKIND_New:
      return true;
    default: return false;
  }
}

static Ast *expr(struct mparser *p) { return expr_assign(p); }

static Ast *func_def(struct mparser *p, bool iter) {
  bool is_operator = false;
  bool is_generic = false;
  int op = -1;

  Vector *typevars = NULL;

  /*
   *  def <T> main(): T
   *      ^^^
   */
  if(skip(p, TKIND_Lt)) {    // <
    is_generic = true;

    typevars = new_vector();

    for(int i = 0; !skip(p, TKIND_Gt); i++) {    // >
      if(i > 0)
        expect(p, TKIND_Comma);
      char *name = curtk(p)->value;
      step(p);
      vec_push(typevars, new_type_variable(name));
    }
  }

  if(curtk(p)->kind == TKIND_BQLIT) {
    is_operator = true;
    op = curtk(p)->cont;

    if(op == -1) {
      error("operators that cannot be overloaded");
    }
  }

  char *name = curtk(p)->value;
  step(p);

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
    do {
      char *name = eat_identifer(p);
      if(!name) {
        skip_to(p, TKIND_Rparen);
        return NULL;
      }
      vec_push(argnames, name);
      if(curtk_is(p, TKIND_Colon)) break;

      expect(p, TKIND_Comma);
    } while(1);

    expect(p, TKIND_Colon);

    Type *cur_ty = eval_type(p);

    for(int i = 0; i < argnames->len; ++i)
      vec_push(argtys, cur_ty);

    for(int i = 0; i < argnames->len; ++i) {
      vec_push(args,
          node_variable_with_type(argnames->data[i], 0, cur_ty));
    }
  }

  /*
   *  def main(): int {
   *            ^^^^^
   *  def main() = expr;
   */
  Type *ret_ty = skip(p, TKIND_Colon) ? eval_type(p) : NULL;
  Type *fntype = iter? new_type_iter(argtys, ret_ty) : new_type_function(argtys, ret_ty);
  Ast *block;

  if(curtk_is(p, TKIND_Lbrace)) { 
    block = make_block(p);
    if(ret_ty == NULL)
      fntype->fnret = mxcty_none;
  }
  else if(curtk_is(p, TKIND_Assign)) {
    step(p);
    block = expr(p);
    expect(p, TKIND_Semicolon);
    if(ret_ty == NULL)
      fntype->fnret = new_type(CTYPE_UNINFERRED);
  }
  else {
    unexpected_token(see(p, 0)->start,
        see(p, 0)->end,
        curtk(p)->value,
        "=", "{", NULL);
    block = NULL;
  }
  NodeVariable *function = node_variable_with_type(name, 0, fntype);
  NodeFunction *node = node_function(function, block, typevars, args, iter);

  if(is_operator)
    node->op = op;

  return (Ast *)node;
}

static Ast *var_decl_block(struct mparser *p, bool isconst) {
  Vector *block = new_vector();

  for(;;) {
    Type *ty = NULL;
    Ast *init = NULL;
    NodeVariable *var = NULL;

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

    var = node_variable_with_type(name, vattr, ty);
    expect(p, TKIND_Semicolon);
    vec_push(block, node_vardecl(var, init, NULL));

    if(skip(p, TKIND_Rbrace)) break;
  }

  return (Ast *)node_vardecl(NULL, NULL, block);
}

static Ast *var_decl(struct mparser *p, bool isconst) {
  if(skip(p, TKIND_Lbrace)) {
    return var_decl_block(p, isconst);
  }

  int vattr = 0;
  if(isconst) vattr |= VARATTR_CONST;

  Ast *init = NULL;
  Type *ty = NULL;
  NodeVariable *var = NULL;
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
    error_at(see(p, 0)->start, see(p, 0)->end, 
        "const must initialize");
    init = NULL;
  }
  else {
    init = NULL;
  }
  var = node_variable_with_type(name, vattr, ty);
  expect(p, TKIND_Semicolon);

  return (Ast *)node_vardecl(var, init, NULL);
}

static Ast *make_object(struct mparser *p) {
  /*
   *  object TagName {
   *      a: int,
   *      b: string
   *  }
   */
  char *tag = fetchtk(p)->value;

  expect(p, TKIND_Lbrace);

  Vector *decls = new_vector();

  for(int i = 0; !skip(p, TKIND_Rbrace); ++i) {
    if(i > 0)
      expect(p, TKIND_Comma);
    char *name = eat_identifer(p);
    if(!name) {
      skip_to(p, TKIND_Rbrace);
      return NULL;
    }

    expect(p, TKIND_Colon);

    Type *ty = eval_type(p);
    vec_push(decls, node_variable_with_type(name, 0, ty));
  }

  return (Ast *)node_object(tag, decls);
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
  Vector *AST = enter(token, p);
  for(int i = 0; i < AST->len; i++) {
    vec_push(s, AST->data[i]);
  }

  return 0;
}

static Ast *make_moduse(struct mparser *p) {
  Vector *mod_names = new_vector();
  Vector *statements = new_vector();
  char *mod = fetchtk(p)->value;

  if(make_ast_from_mod(p, statements, mod))
    return NULL;

  NodeBlock *block = node_block(statements);
  expect(p, TKIND_Semicolon);

  return (Ast *)node_namespace(mod, block);
}

static Ast *make_breakpoint(struct mparser *p) {
  Ast *a = (Ast *)node_breakpoint();
  expect(p, TKIND_Semicolon);

  return a;
}

static Ast *make_assert(struct mparser *p) {
  Ast *a = expr(p);
  NodeAssert *as = node_assert(a);
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
    ty = mxcty_int;
  else if(skip(p, TKIND_TUint))
    ty = new_type(CTYPE_UINT);
  else if(skip(p, TKIND_TBool))
    ty = mxcty_bool;
  else if(skip(p, TKIND_TString))
    ty = mxcty_string;
  else if(skip(p, TKIND_TChar))
    ty = mxcty_char;
  else if(skip(p, TKIND_TFloat))
    ty = mxcty_float;
  else if(skip(p, TKIND_TFile))
    ty = mxcty_file;
  else if(skip(p, TKIND_TNone)) // TODO :only function rettype
    ty = mxcty_none;
  else if(skip(p, TKIND_Fn)) {
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
      ty = new_type_ptr(ty);
    else
      break;
  }
  /*
   *  int?
   *     ^
   */
  if(skip(p, TKIND_Question))
    ty = (Type *)New_MxcOptional(ty);

  return ty;
}

Ast *make_assign(Ast *dst, Ast *src) {
  if(!dst) return NULL;
  return (Ast *)node_assign(dst, src);
}

Ast *make_assigneq(char *op, Ast *dst, Ast *src) {
  (void)op;
  (void)dst;
  (void)src;
  return NULL; // TODO
}

static Ast *make_block(struct mparser *p) {
  expect(p, TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;
  for(;;) {
    if(skip(p, TKIND_Rbrace))
      break;
    b = statement(p);

    if(ast_isexpr(b)) {
      expect(p, TKIND_Semicolon);
    }
    vec_push(cont, b);
  }

  return (Ast *)node_block(cont);
}

static Ast *make_typed_block(struct mparser *p) {
  expect(p, TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;

  for(;;) {
    if(skip(p, TKIND_Rbrace))
      break;
    b = statement(p);

    if(ast_isexpr(b)) {
      expect(p, TKIND_Semicolon);
    }
    vec_push(cont, b);
  }

  return (Ast *)node_typedblock(cont);
}

static Ast *make_if(struct mparser *p, bool isexpr) {
  Ast *cond = expr(p);
  Ast *then = isexpr ? expr(p) : make_block(p);

  if(skip(p, TKIND_Else)) {
    Ast *el;

    if(skip(p, TKIND_If))
      el = make_if(p, isexpr);
    else
      el = isexpr ? expr(p) : make_block(p);

    return (Ast *)node_if(cond, then, el, isexpr);
  }
  return (Ast *)node_if(cond, then, NULL, isexpr);
}

static Ast *make_for(struct mparser *p) {
  /*
   *  for i in iter() { }
   */

  Vector *v = new_vector();

  do {
    if(curtk(p)->kind == TKIND_Identifer) {
      Type *ty = new_type(CTYPE_UNINFERRED); 
      vec_push(v,
          node_variable_with_type(curtk(p)->value, 0, ty));
    }
    else {
      error_at(see(p, 0)->start, see(p, 0)->end, "expected identifer");
    }

    step(p);
  } while(!skip(p, TKIND_In));

  Ast *iter = expr(p);
  if(!iter) return NULL;

  Ast *body = (Ast *)make_block(p);
  if(!body) return NULL;

  return (Ast *)node_for(v, iter, body);
}

static Ast *make_while(struct mparser *p) {
  Ast *cond = expr(p);
  Ast *body = make_block(p);

  return (Ast *)node_while(cond, body);
}

static Ast *make_return(struct mparser *p) {
  Ast *e = expr(p);
  NodeReturn *ret = node_return(e);
  if(e->type != NDTYPE_NONENODE)
    expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_yield(struct mparser *p) {
  Ast *e = expr(p);
  NodeYield *ret = node_yield(e);
  if(e->type != NDTYPE_NONENODE)
    expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_break(struct mparser *p) {
  NodeBreak *ret = node_break();
  expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_skip(struct mparser *p) {
  NodeSkip *ret = node_skip();
  expect(p, TKIND_Semicolon);

  return (Ast *)ret;
}

static void make_typedef(struct mparser *p) {
  mxc_unimplemented("typedef");
}

static Ast *expr_char(struct mparser *p) {
  Token *cur = fetchtk(p);
  return (Ast *)node_char(cur->cont);
}

static Ast *expr_num(Token *tk) {
  if(strchr(tk->value, '.'))
    return (Ast *)node_number_float(atof(tk->value));

  int overflow = 0;
  size_t len;
  int64_t i = intern_scan_digiti(tk->value, 10, &overflow, &len);
  if(overflow) {
    MxcValue a = new_integer(tk->value, 10);
    return (Ast *)node_number_big(a);
  }
  return (Ast *)node_number_int(i);
}

static Ast *expr_string(Token *tk) { return (Ast *)node_string(tk->value); }

static Ast *expr_var(struct mparser *p) {
  char *name = eat_identifer(p);
  if(!name) return NULL;

  return (Ast *)node_variable(name, 0);
}

static Ast *expr_catcherr(struct mparser *p);

static Ast *expr_assign(struct mparser *p) {
  Ast *left = expr_catcherr(p);
  if(curtk_is(p, TKIND_Assign)) {
    step(p);
    left = make_assign(left, expr_assign(p));
  }
  else if(curtk_is(p, TKIND_PlusAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_ADD, left, expr_assign(p)));
  }
  else if(curtk_is(p, TKIND_MinusAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_SUB, left, expr_assign(p)));
  }
  else if(curtk_is(p, TKIND_AsteriskAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_MUL, left, expr_assign(p)));
  }
  else if(curtk_is(p, TKIND_DivAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_DIV, left, expr_assign(p)));
  }
  else if(curtk_is(p, TKIND_ModAs)) {
    step(p);
    left = make_assign(left, (Ast *)node_binary(BIN_MOD, left, expr_assign(p)));
  }

  return left;
}

static Ast *expr_range(struct mparser *);

static Ast *expr_catcherr(struct mparser *p) {
  Ast *left = expr_range(p);
  Ast *r;

  for(;;) {
    if(curtk_is(p, TKIND_Question)) {
      step(p);
      r = expr_range(p);
      left = (Ast *)node_binary(BIN_QUESTION, left, r);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_range(struct mparser *p) {
  Ast *left = expr_logic_or(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_DotDot)) {
      step(p);
      t = expr_logic_or(p);
      left = (Ast *)node_binary(BIN_DOTDOT, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_or(struct mparser *p) {
  Ast *left = expr_logic_and(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_LogOr) || curtk_is(p, TKIND_KOr)) {
      step(p);
      t = expr_logic_and(p);
      left = (Ast *)node_binary(BIN_LOR, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_and(struct mparser *p) {
  Ast *left = expr_bin_xor(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_LogAnd) || curtk_is(p, TKIND_KAnd)) {
      step(p);
      t = expr_bin_xor(p);
      left = (Ast *)node_binary(BIN_LAND, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_bin_xor(struct mparser *p) {
  Ast *left = expr_equality(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Xor)) {
      step(p);
      t = expr_equality(p);
      left = (Ast *)node_binary(BIN_BXOR, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_equality(struct mparser *p) {
  Ast *left = expr_comp(p);

  for(;;) {
    if(curtk_is(p, TKIND_Eq)) {
      step(p);
      Ast *t = expr_comp(p);
      left = (Ast *)node_binary(BIN_EQ, left, t);
    }
    else if(curtk_is(p, TKIND_Neq)) {
      step(p);
      Ast *t = expr_comp(p);
      left = (Ast *)node_binary(BIN_NEQ, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_comp(struct mparser *p) {
  Ast *left = expr_bitshift(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Lt)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_LT, left, t);
    }
    else if(curtk_is(p, TKIND_Gt)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_GT, left, t);
    }
    else if(curtk_is(p, TKIND_Lte)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_LTE, left, t);
    }
    else if(curtk_is(p, TKIND_Gte)) {
      step(p);
      t = expr_bitshift(p);
      left = (Ast *)node_binary(BIN_GTE, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_bitshift(struct mparser *p) {
  Ast *left = expr_add(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Lshift)) {
      step(p);
      t = expr_add(p);
      left = (Ast *)node_binary(BIN_LSHIFT, left, t);
    }
    else if(curtk_is(p, TKIND_Rshift)) {
      step(p);
      t = expr_add(p);
      left = (Ast *)node_binary(BIN_RSHIFT, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_add(struct mparser *p) {
  Ast *left = expr_mul(p);

  for(;;) {
    if(curtk_is(p, TKIND_Plus)) {
      step(p);
      Ast *t = expr_mul(p);
      left = (Ast *)node_binary(BIN_ADD, left, t);
    }
    else if(curtk_is(p, TKIND_Minus)) {
      step(p);
      Ast *t = expr_mul(p);
      left = (Ast *)node_binary(BIN_SUB, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_mul(struct mparser *p) {
  Ast *left = expr_unary(p);
  Ast *t;

  for(;;) {
    if(curtk_is(p, TKIND_Asterisk)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_MUL, left, t);
    }
    else if(curtk_is(p, TKIND_Div)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_DIV, left, t);
    }
    else if(curtk_is(p, TKIND_Mod)) {
      step(p);
      t = expr_unary(p);
      left = (Ast *)node_binary(BIN_MOD, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_unary(struct mparser *p) {
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

  return (Ast *)node_unary(op, operand);
}

static Ast *expr_unary_postfix(struct mparser *p) {
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

        for(int i = 0; !skip(p, TKIND_Rparen); ++i) {
          if(i > 0) {
            expect(p, TKIND_Comma);
          }

          vec_push(args, expr(p));
        }

        left = (Ast *)node_fncall(memb, args);
      }
      else if(is_expr_tk(p)) {
        Vector *args = new_vector();
        vec_push(args, left);
        do {
          vec_push(args, expr(p));
        } while(skip(p, TKIND_Comma));
        left = (Ast *)node_fncall(memb, args);
      }
      else { // struct
        left = (Ast *)node_dotexpr(left, memb);
      }
    }
    else if(curtk_is(p, TKIND_Lboxbracket)) {
      step(p);
      Ast *index = expr(p);
      expect(p, TKIND_Rboxbracket);

      left = (Ast *)node_subscript(left, index);
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
      left = (Ast *)node_fncall(left, args);
    }
    else if(is_expr_tk(p)) {
      Vector *args = new_vector();

      do {
        vec_push(args, expr(p));
      } while(skip(p, TKIND_Comma));

      left = (Ast *)node_fncall(left, args);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_unary_postfix_atmark(struct mparser *p) {
  Ast *left = expr_primary(p);

  for(;;) {
    if(curtk_is(p, TKIND_Atmark)) {
      step(p);
      Ast *ident = expr_var(p);

      left = (Ast *)node_modulefunccall(left, ident);
    }
    else {
      return left;
    }
  }
}

static Ast *make_hash(struct mparser *p) {
  /*
   *   let table = #["a":100, "b":200];
   *   let niltable = #;
   */
  /* TODO */
  return NULL;
}

static Ast *expr_primary(struct mparser *p) {
  if(skip(p, TKIND_True)) {
    return (Ast *)node_bool(true);
  }
  else if(skip(p, TKIND_False)) {
    return (Ast *)node_bool(false);
  }
  else if(skip(p, TKIND_Null)) {
    return (Ast *)node_null();
  }
  else if(skip(p, TKIND_New)) {
    return new_object(p);
  }
  else if(skip(p, TKIND_If)) {
    return make_if(p, true);
  }
  else if(skip(p, TKIND_Hash)) {
    return make_hash(p);
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
  else if(curtk_is(p, TKIND_Char)) {
    return expr_char(p);
  }
  else if(skip(p, TKIND_Lparen)) {
    if(skip(p, TKIND_Rparen))
      return NULL;

    Ast *left = expr(p);
    if(skip(p, TKIND_Comma)) { // tuple
      if(skip(p, TKIND_Rparen)) {
        error("error"); // TODO
        return NULL;
      }
      Vector *exs = new_vector();
      Ast *a;
      Type *ty = new_type(CTYPE_TUPLE);
      vec_push(exs, left);

      vec_push(ty->tuple, left->ctype);

      for(;;) {
        a = expr(p);
        vec_push(ty->tuple, a->ctype);
        vec_push(exs, a);
        if(skip(p, TKIND_Rparen))
          return (Ast *)node_tuple(exs, exs->len, ty);
        expect(p, TKIND_Comma);
      }
    }

    if(!expect(p, TKIND_Rparen))
      step(p);

    return left;
  }
  else if(curtk_is(p, TKIND_Lboxbracket)) {
    step(p);
    Vector *elem = new_vector();
    Ast *a;

    for(int i = 0; !skip(p, TKIND_Rboxbracket); ++i) {
      if(i > 0) {
        if(skip(p, TKIND_Semicolon)) {
          del_vector(elem);
          return make_list_with_size(p, a);
        }
        expect(p, TKIND_Comma);
      }
      a = expr(p);
      vec_push(elem, a);
    }

    return (Ast *)node_list(elem, elem->len, NULL, NULL);
  }
  else if(curtk_is(p, TKIND_Semicolon)) {
    step(p);
    return NONE_NODE;
  }
  else if(curtk_is(p, TKIND_End)) {
    return NULL;
  }
  error_at(see(p, 0)->start, see(p, 0)->end, "syntax error");
  skip_to(p, TKIND_Semicolon);

  return NULL;
}

static Ast *make_list_with_size(struct mparser *p, Ast *nelem) {
  Ast *init = expr(p);
  expect(p, TKIND_Rboxbracket);

  return (Ast *)node_list(NULL, 0, nelem, init); 
}

static Ast *new_object(struct mparser *p) {
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

    for(int i = 0; !skip(p, TKIND_Rbrace); ++i) {
      if(i > 0)
        expect(p, TKIND_Comma);
      vec_push(fields, expr_var(p));
      expect(p, TKIND_Colon);

      vec_push(inits, expr(p));
    }
  }

  return (Ast *)node_struct_init(tag, fields, inits);
}

static Ast *statement(struct mparser *p) {
  if(curtk_is(p, TKIND_Lbrace)) {
    return make_block(p);
  }
  else if(skip(p, TKIND_For)) {
    return make_for(p);
  }
  else if(skip(p, TKIND_While)) {
    return make_while(p);
  }
  else if(skip(p, TKIND_If)) {
    return make_if(p, false);
  }
  else if(skip(p, TKIND_Return)) {
    return make_return(p);
  }
  else if(skip(p, TKIND_YIELD)) {
    return make_yield(p);
  }
  else if(skip(p, TKIND_Break)) {
    return make_break(p);
  }
  else if(skip(p, TKIND_Skip)) {
    return make_skip(p);
  }
  else if(skip(p, TKIND_Let)) {
    return var_decl(p, false);
  }
  else if(skip(p, TKIND_Const)) {
    return var_decl(p, true);
  }
  else if(skip(p, TKIND_ITERATOR)) {
    return func_def(p, true);
  }
  else if(skip(p, TKIND_Fn)) {
    return func_def(p, false);
  }
  else if(skip(p, TKIND_Object)) {
    return make_object(p);
  }
  else if(skip(p, TKIND_Use)) {
    return make_moduse(p);
  }
  else if(skip(p, TKIND_BreakPoint)) {
    return make_breakpoint(p);
  }
  else if(skip(p, TKIND_Assert)) {
    return make_assert(p);
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

