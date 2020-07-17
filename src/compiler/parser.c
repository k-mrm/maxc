#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "internal.h"
#include "ast.h"
#include "error/error.h"
#include "lexer.h"
#include "object/integerobject.h"

static Ast *make_block(void);
static Ast *statement(void);
static Ast *expr(void);
static Ast *expr_assign(void);
static Ast *expr_equality(void);
static Ast *expr_logic_or(void);
static Ast *expr_bin_xor(void);
static Ast *expr_logic_and(void);
static Ast *expr_comp(void);
static Ast *expr_bitshift(void);
static Ast *expr_add(void);
static Ast *expr_mul(void);
static Ast *expr_unary(void);
static Ast *expr_unary_postfix(void);
static Ast *expr_unary_postfix_atmark(void);
static Ast *expr_primary(void);
static Ast *new_object(void);
static Ast *make_list_with_size(Ast *);
static Ast *var_decl(bool);
static Ast *expr_char();
static Ast *expr_num(Token *);
static Ast *expr_unary(void);
static Type *eval_type(void);
static Token *see(int);
static Vector *enter(Vector *);

static Vector *tokens = NULL;
static Vector *tokens_stack;
static Vector *pos_stack;
static int pos = 0;
static int nenter = 0;

#define Step() (++pos)
#define Cur_Token() ((Token *)tokens->data[pos])
#define Get_Step_Token() ((Token *)tokens->data[pos++])
#define Cur_Token_Is(tk) ((Cur_Token()->kind) == (tk))

#define Get_Cur_Line() (Cur_Token()->start.line)

struct mparser {
};

static bool skip(enum tkind tk) {
  if(Cur_Token()->kind == tk) {
    ++pos;
    return true;
  }
  return false;
}

static bool skip2(enum tkind tk1, enum tkind tk2) {
  int tmp = pos;
  if(Cur_Token()->kind == tk1) {
    ++pos;
    if(Cur_Token()->kind == tk2) {
      ++pos;
      return true;
    }
  }

  pos = tmp;
  return false;
}

static Token *expect(enum tkind tk) {
  Token *cur = Cur_Token();
  if(cur->kind == tk) {
    ++pos;
    return cur;
  }
  else {
    expect_token(see(-1)->start, see(-1)->end, tk2str(tk));
    return NULL;
  }
}

static Token *expect_type(enum tkind tk) {
  Token *cur = Cur_Token();
  if(cur->kind == tk) {
    ++pos;
    return cur;
  }
  else {
    expect_token(see(0)->start, see(0)->end, tk2str(tk));
    return NULL;
  }
}

static char *eat_identifer() {
  Token *tk = Get_Step_Token();
  if(tk->kind != TKIND_Identifer) {
    unexpected_token(tk->start,
        tk->end,
        tk->value,
        "Identifer", NULL);
    return NULL;
  }
  return tk->value;
}

static void skip_to(enum tkind tk) {
  while(!Cur_Token_Is(tk) && !Cur_Token_Is(TKIND_End)) {
    Step();
  }
}

static Token *see(int p) { return tokens->data[pos + p]; }

static bool is_expr_tk() {
  switch(Cur_Token()->kind) {
    case TKIND_Num: case TKIND_String:
    case TKIND_Char: case TKIND_Identifer:
    case TKIND_Lboxbracket: case TKIND_Bang:
    case TKIND_True: case TKIND_False: case TKIND_New:
      return true;
    default: return false;
  }
}

static Ast *expr() { return expr_assign(); }

static Ast *func_def(bool iter) {
  bool is_operator = false;
  bool is_generic = false;
  int op = -1;

  Vector *typevars = NULL;

  /*
   *  def <T> main(): T
   *      ^^^
   */
  if(skip(TKIND_Lt)) {    // <
    is_generic = true;

    typevars = new_vector();

    for(int i = 0; !skip(TKIND_Gt); i++) {    // >
      if(i > 0) {
        expect(TKIND_Comma);
      }
      char *name = Cur_Token()->value;
      Step();
      vec_push(typevars, new_type_variable(name));
    }
  }

  if(Cur_Token()->kind == TKIND_BQLIT) {
    is_operator = true;
    op = Cur_Token()->cont;

    if(op == -1) {
      error("operators that cannot be overloaded");
    }
  }

  char *name = Cur_Token()->value;
  Step();

  // def main(): typename {
  //         ^
  if(!expect(TKIND_Lparen)) {
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
  for(int i = 0; !skip(TKIND_Rparen); i++) {
    if(i > 0) {
      expect(TKIND_Comma);
    }
    Vector *argnames = new_vector();
    do {
      char *name = eat_identifer();
      if(!name) {
        skip_to(TKIND_Rparen);
        return NULL;
      }
      vec_push(argnames, name);
      if(Cur_Token_Is(TKIND_Colon)) break;

      expect(TKIND_Comma);
    } while(1);

    expect(TKIND_Colon);

    Type *cur_ty = eval_type();

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
  Type *ret_ty = skip(TKIND_Colon) ? eval_type() : NULL;
  Type *fntype = new_type_function(argtys, ret_ty);
  Ast *block;

  if(Cur_Token_Is(TKIND_Lbrace)) { 
    block = make_block();
    if(ret_ty == NULL)
      fntype->fnret = mxcty_none;
  }
  else if(Cur_Token_Is(TKIND_Assign)) {
    Step();
    block = expr();
    expect(TKIND_Semicolon);
    if(ret_ty == NULL)
      fntype->fnret = new_type(CTYPE_UNINFERRED);
  }
  else {
    unexpected_token(see(0)->start,
        see(0)->end,
        Cur_Token()->value,
        "=", "{", NULL);
    block = NULL;
  }
  NodeVariable *function = node_variable_with_type(name, 0, fntype);
  NodeFunction *node = node_function(function, block, typevars, args, iter);

  if(is_operator) {
    node->op = op;
  }

  return (Ast *)node;
}

static Ast *var_decl_block(bool isconst) {
  Vector *block = new_vector();

  for(;;) {
    Type *ty = NULL;
    Ast *init = NULL;
    NodeVariable *var = NULL;

    char *name = eat_identifer();
    if(!name) {
      skip_to(TKIND_Rbrace);
      return NULL;
    }

    if(skip(TKIND_Colon)) {
      ty = eval_type();
    }
    else {
      ty = new_type(CTYPE_UNINFERRED);
    }
    int vattr = 0;

    if(isconst) {
      vattr |= VARATTR_CONST;
    }

    if(skip(TKIND_Assign)) {
      init = expr();
    }
    else if(isconst) {
      error_at(see(0)->start, see(0)->end, "const must initialize");
    }

    var = node_variable_with_type(name, vattr, ty);
    expect(TKIND_Semicolon);
    vec_push(block, node_vardecl(var, init, NULL));

    if(skip(TKIND_Rbrace)) break;
  }

  return (Ast *)node_vardecl(NULL, NULL, block);
}

static Ast *var_decl(bool isconst) {
  if(skip(TKIND_Lbrace)) {
    return var_decl_block(isconst);
  }

  int vattr = 0;
  if(isconst)
    vattr |= VARATTR_CONST;

  Ast *init = NULL;
  Type *ty = NULL;
  NodeVariable *var = NULL;
  char *name = eat_identifer();
  if(!name) {
    skip_to(TKIND_Semicolon);
    return NULL;
  }
  /*
   *  let a(: int) = 100;
   *        ^^^^^
   */
  ty = skip(TKIND_Colon) ? eval_type() : new_type(CTYPE_UNINFERRED);
  /*
   *  let a: int = 100;
   *             ^
   */
  if(skip(TKIND_Assign)) {
    init = expr();
    if(!init) return NULL;
  }
  else if(isconst) {
    error_at(see(0)->start, see(0)->end, 
        "const must initialize");
    init = NULL;
  }
  else {
    init = NULL;
  }
  var = node_variable_with_type(name, vattr, ty);
  expect(TKIND_Semicolon);

  return (Ast *)node_vardecl(var, init, NULL);
}

static Ast *make_object() {
  /*
   *  object TagName {
   *      a: int,
   *      b: string
   *  }
   */
  char *tag = Get_Step_Token()->value;

  expect(TKIND_Lbrace);

  Vector *decls = new_vector();

  for(int i = 0; !skip(TKIND_Rbrace); ++i) {
    if(i > 0) {
      expect(TKIND_Comma);
    }
    char *name = eat_identifer();
    if(!name) {
      skip_to(TKIND_Rbrace);
      return NULL;
    }

    expect(TKIND_Colon);

    Type *ty = eval_type();
    vec_push(decls, node_variable_with_type(name, 0, ty));
  }

  return (Ast *)node_object(tag, decls);
}

static int make_ast_from_mod(Vector *s, char *name) {
  char path[512];
  sprintf(path, "./lib/%s.mxc", name);
  char *src = read_file(path);
  if(!src) {
    memset(path, 0, 512);
    sprintf(path, "./%s.mxc", name);

    src = read_file(path);
    if(!src) {
      error_at(see(-1)->start, see(-1)->end, "lib %s: not found", name);
      return 1;
    }
  }

  Vector *token = lexer_run(src, name);
  Vector *AST = enter(token);
  for(int i = 0; i < AST->len; i++) {
    vec_push(s, AST->data[i]);
  }

  return 0;
}

static Ast *make_moduse() {
  Vector *mod_names = new_vector();
  Vector *statements = new_vector();
  char *mod = Get_Step_Token()->value;

  if(make_ast_from_mod(statements, mod)) {
    return NULL;
  }

  NodeBlock *block = node_block(statements);
  expect(TKIND_Semicolon);

  return (Ast *)node_namespace(mod, block);
}

static Ast *make_breakpoint() {
  Ast *a = (Ast *)node_breakpoint();
  expect(TKIND_Semicolon);

  return a;
}

static Ast *make_assert() {
  Ast *a = expr();
  NodeAssert *ass = node_assert(a);
  expect(TKIND_Semicolon);

  return (Ast *)ass;
}

static Type *eval_type() {
  Type *ty;

  if(skip(TKIND_Lparen)) { // tuple
    ty = new_type(CTYPE_TUPLE);

    for(;;) {
      vec_push(ty->tuple, eval_type());

      if(skip(TKIND_Rparen))
        break;

      expect(TKIND_Comma);
    }
  }
  else if(skip(TKIND_TInt))
    ty = mxcty_int;
  else if(skip(TKIND_TUint))
    ty = new_type(CTYPE_UINT);
  else if(skip(TKIND_TBool))
    ty = mxcty_bool;
  else if(skip(TKIND_TString))
    ty = mxcty_string;
  else if(skip(TKIND_TChar))
    ty = mxcty_char;
  else if(skip(TKIND_TFloat))
    ty = mxcty_float;
  else if(skip(TKIND_TFile))
    ty = mxcty_file;
  else if(skip(TKIND_TNone)) // TODO :only function rettype
    ty = mxcty_none;
  else if(skip(TKIND_Fn)) {
    expect(TKIND_Lparen);
    Vector *arg = new_vector();

    while(!skip(TKIND_Rparen)) {
      vec_push(arg, eval_type());
      if(skip(TKIND_Rparen))
        break;
      expect(TKIND_Comma);
    }
    expect(TKIND_Colon);
    Type *ret = eval_type();
    ty = new_type_function(arg, ret);
  }
  else {
    char *tk = Cur_Token()->value;
    Step();
    ty = new_type_unsolved(tk);
  }

  for(;;) {
    if(skip2(TKIND_Lboxbracket, TKIND_Rboxbracket))
      ty = new_type_ptr(ty);
    else
      break;
  }
  /*
   *  int?
   *     ^
   */
  if(skip(TKIND_Question)) {
    ty = (Type *)New_MxcOptional(ty);
  }

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

static Ast *make_block() {
  expect(TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;
  for(;;) {
    if(skip(TKIND_Rbrace))
      break;
    b = statement();

    if(ast_isexpr(b)) {
      expect(TKIND_Semicolon);
    }
    vec_push(cont, b);
  }

  return (Ast *)node_block(cont);
}

static Ast *make_typed_block() {
  expect(TKIND_Lbrace);
  Vector *cont = new_vector();
  Ast *b;

  for(;;) {
    if(skip(TKIND_Rbrace))
      break;
    b = statement();

    if(ast_isexpr(b)) {
      expect(TKIND_Semicolon);
    }
    vec_push(cont, b);
  }

  return (Ast *)node_typedblock(cont);
}

static Ast *make_if(bool isexpr) {
  Ast *cond = expr();
  Ast *then = isexpr ? expr() : make_block();

  if(skip(TKIND_Else)) {
    Ast *el;

    if(skip(TKIND_If))
      el = make_if(isexpr);
    else
      el = isexpr ? expr() : make_block();

    return (Ast *)node_if(cond, then, el, isexpr);
  }
  return (Ast *)node_if(cond, then, NULL, isexpr);
}

static Ast *make_for() {
  /*
   *  for i in iter() { }
   */

  Vector *v = new_vector();

  do {
    if(Cur_Token()->kind == TKIND_Identifer) {
      Type *ty = new_type(CTYPE_UNINFERRED); 
      vec_push(v,
          node_variable_with_type(Cur_Token()->value, 0, ty));
    }
    else {
      error_at(see(0)->start, see(0)->end,
          "expected identifer");
    }

    Step();
  } while(!skip(TKIND_In));

  Ast *iter = expr();
  if(!iter) {
    return NULL;
  }
  Ast *body = (Ast *)make_block();
  if(!body) {
    return NULL;
  }

  return (Ast *)node_for(v, iter, body);
}

static Ast *make_while() {
  Ast *cond = expr();
  Ast *body = make_block();

  return (Ast *)node_while(cond, body);
}

static Ast *make_return() {
  Ast *e = expr();
  NodeReturn *ret = node_return(e);
  if(e->type != NDTYPE_NONENODE)
    expect(TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_yield() {
  Ast *e = expr();
  NodeYield *ret = node_yield(e);
  if(e->type != NDTYPE_NONENODE)
    expect(TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_break() {
  NodeBreak *ret = node_break();
  expect(TKIND_Semicolon);

  return (Ast *)ret;
}

static Ast *make_skip() {
  NodeSkip *ret = node_skip();
  expect(TKIND_Semicolon);

  return (Ast *)ret;
}

static void make_typedef() {
  mxc_unimplemented("typedef");
}

static Ast *expr_char() {
  Token *cur = Get_Step_Token();
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

static Ast *expr_var() {
  char *name = eat_identifer();
  if(!name) return NULL;

  return (Ast *)node_variable(name, 0);
}

static Ast *expr_range();

static Ast *expr_assign() {
  Ast *left = expr_range();
  if(Cur_Token_Is(TKIND_Assign)) {
    if(left == NULL) {
      return NULL;
    }

    Step();
    left = make_assign(left, expr_assign());
  }

  return left;
}

static Ast *expr_range() {
  Ast *left = expr_logic_or();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_DotDot)) {
      Step();
      t = expr_logic_or();
      left = (Ast *)node_binary(BIN_DOTDOT, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_or() {
  Ast *left = expr_logic_and();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_LogOr) || Cur_Token_Is(TKIND_KOr)) {
      Step();
      t = expr_logic_and();
      left = (Ast *)node_binary(BIN_LOR, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_logic_and() {
  Ast *left = expr_bin_xor();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_LogAnd) || Cur_Token_Is(TKIND_KAnd)) {
      Step();
      t = expr_bin_xor();
      left = (Ast *)node_binary(BIN_LAND, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_bin_xor() {
  Ast *left = expr_equality();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_Xor)) {
      Step();
      t = expr_equality();
      left = (Ast *)node_binary(BIN_BXOR, left, t);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_equality() {
  Ast *left = expr_comp();

  for(;;) {
    if(Cur_Token_Is(TKIND_Eq)) {
      Step();
      Ast *t = expr_comp();
      left = (Ast *)node_binary(BIN_EQ, left, t);
    }
    else if(Cur_Token_Is(TKIND_Neq)) {
      Step();
      Ast *t = expr_comp();
      left = (Ast *)node_binary(BIN_NEQ, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_comp() {
  Ast *left = expr_bitshift();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_Lt)) {
      Step();
      t = expr_bitshift();
      left = (Ast *)node_binary(BIN_LT, left, t);
    }
    else if(Cur_Token_Is(TKIND_Gt)) {
      Step();
      t = expr_bitshift();
      left = (Ast *)node_binary(BIN_GT, left, t);
    }
    else if(Cur_Token_Is(TKIND_Lte)) {
      Step();
      t = expr_bitshift();
      left = (Ast *)node_binary(BIN_LTE, left, t);
    }
    else if(Cur_Token_Is(TKIND_Gte)) {
      Step();
      t = expr_bitshift();
      left = (Ast *)node_binary(BIN_GTE, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_bitshift() {
  Ast *left = expr_add();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_Lshift)) {
      Step();
      t = expr_add();
      left = (Ast *)node_binary(BIN_LSHIFT, left, t);
    }
    else if(Cur_Token_Is(TKIND_Rshift)) {
      Step();
      t = expr_add();
      left = (Ast *)node_binary(BIN_RSHIFT, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_add() {
  Ast *left = expr_mul();

  for(;;) {
    if(Cur_Token_Is(TKIND_Plus)) {
      Step();
      Ast *t = expr_mul();
      left = (Ast *)node_binary(BIN_ADD, left, t);
    }
    else if(Cur_Token_Is(TKIND_Minus)) {
      Step();
      Ast *t = expr_mul();
      left = (Ast *)node_binary(BIN_SUB, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_mul() {
  Ast *left = expr_unary();
  Ast *t;

  for(;;) {
    if(Cur_Token_Is(TKIND_Asterisk)) {
      Step();
      t = expr_unary();
      left = (Ast *)node_binary(BIN_MUL, left, t);
    }
    else if(Cur_Token_Is(TKIND_Div)) {
      Step();
      t = expr_unary();
      left = (Ast *)node_binary(BIN_DIV, left, t);
    }
    else if(Cur_Token_Is(TKIND_Mod)) {
      Step();
      t = expr_unary();
      left = (Ast *)node_binary(BIN_MOD, left, t);
    }
    else
      return left;
  }
}

static Ast *expr_unary() {
  enum tkind tk = Cur_Token()->kind;
  enum UNAOP op = -1;

  switch(tk) {
    case TKIND_Minus:
      op = UNA_MINUS;
      break;
    case TKIND_Bang:
      op = UNA_NOT;
      break;
    default:
      return expr_unary_postfix();
  }

  Step();
  Ast *operand = expr_unary();
  if(operand == NONE_NODE) {
    error_at(see(-1)->start, see(-1)->end,
        "expected expression before `;`");
    return NULL;
  }

  return (Ast *)node_unary(op, operand);
}

static Ast *expr_unary_postfix() {
  Ast *left = expr_unary_postfix_atmark();

  for(;;) {
    if(Cur_Token_Is(TKIND_Dot)) {
      Step();

      Ast *memb = expr_unary_postfix_atmark();
      /*
       *  a.function();
       *            ^
       */
      if(skip(TKIND_Lparen)) {
        Vector *args = new_vector();
        vec_push(args, left);

        for(int i = 0; !skip(TKIND_Rparen); ++i) {
          if(i > 0) {
            expect(TKIND_Comma);
          }

          vec_push(args, expr());
        }

        left = (Ast *)node_fncall(memb, args, NULL);
      }
      else if(is_expr_tk()) {
        Vector *args = new_vector();
        vec_push(args, left);
        do {
          vec_push(args, expr());
        } while(skip(TKIND_Comma));
        left = (Ast *)node_fncall(memb, args, NULL);
      }
      else { // struct
        left = (Ast *)node_dotexpr(left, memb);
      }
    }
    else if(Cur_Token_Is(TKIND_Lboxbracket)) {
      Step();
      Ast *index = expr();
      expect(TKIND_Rboxbracket);

      left = (Ast *)node_subscript(left, index);
    }
    else if(Cur_Token_Is(TKIND_Lparen)) {
      Step();
      Vector *args = new_vector();

      if(skip(TKIND_Rparen)) {}
      else {
        for(;;) {
          vec_push(args, expr());
          if(skip(TKIND_Rparen))
            break;
          expect(TKIND_Comma);
        }
      }
      left = (Ast *)node_fncall(left, args, NULL);
    }
    else if(is_expr_tk()) {
      Vector *args = new_vector();

      do {
        vec_push(args, expr());
      } while(skip(TKIND_Comma));

      left = (Ast *)node_fncall(left, args, NULL);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_unary_postfix_atmark() {
  Ast *left = expr_primary();

  for(;;) {
    if(Cur_Token_Is(TKIND_Atmark)) {
      Step();
      Ast *ident = expr_var();

      left = (Ast *)node_modulefunccall(left, ident);
    }
    else {
      return left;
    }
  }
}

static Ast *expr_primary() {
  if(skip(TKIND_True)) {
    return (Ast *)node_bool(true);
  }
  else if(skip(TKIND_False)) {
    return (Ast *)node_bool(false);
  }
  else if(skip(TKIND_Null)) {
    return (Ast *)node_null();
  }
  else if(skip(TKIND_New)) {
    return new_object();
  }
  else if(skip(TKIND_If)) {
    return make_if(true);
  }
  else if(Cur_Token_Is(TKIND_Identifer)) {
    Ast *v = expr_var();
    return v;
  }
  else if(Cur_Token_Is(TKIND_Num)) {
    return expr_num(Get_Step_Token());
  }
  else if(Cur_Token_Is(TKIND_String)) {
    return expr_string(Get_Step_Token());
  }
  else if(Cur_Token_Is(TKIND_Char)) {
    return expr_char();
  }
  else if(skip(TKIND_Lparen)) {
    if(skip(TKIND_Rparen)) {
      return NULL;
    }

    Ast *left = expr();
    if(skip(TKIND_Comma)) { // tuple
      if(skip(TKIND_Rparen)) {
        error("error"); // TODO
        return NULL;
      }
      Vector *exs = new_vector();
      Ast *a;
      Type *ty = new_type(CTYPE_TUPLE);
      vec_push(exs, left);

      vec_push(ty->tuple, left->ctype);

      for(;;) {
        a = expr();
        vec_push(ty->tuple, a->ctype);
        vec_push(exs, a);
        if(skip(TKIND_Rparen))
          return (Ast *)node_tuple(exs, exs->len, ty);
        expect(TKIND_Comma);
      }
    }

    if(!expect(TKIND_Rparen))
      Step();

    return left;
  }
  else if(Cur_Token_Is(TKIND_Lboxbracket)) {
    Step();
    Vector *elem = new_vector();
    Ast *a;

    for(int i = 0; !skip(TKIND_Rboxbracket); ++i) {
      if(i > 0) {
        if(skip(TKIND_Semicolon)) {
          del_vector(elem);
          return make_list_with_size(a);
        }
        expect(TKIND_Comma);
      }
      a = expr();
      vec_push(elem, a);
    }

    return (Ast *)node_list(elem, elem->len, NULL, NULL);
  }
  else if(Cur_Token_Is(TKIND_Semicolon)) {
    Step();
    return NONE_NODE;
  }
  else if(Cur_Token_Is(TKIND_End)) {
    return NULL;
  }
  error_at(see(0)->start, see(0)->end, "syntax error");
  skip_to(TKIND_Semicolon);

  return NULL;
}

static Ast *make_list_with_size(Ast *nelem) {
  Ast *init = expr();
  expect(TKIND_Rboxbracket);

  return (Ast *)node_list(NULL, 0, nelem, init); 
}

static Ast *new_object() {
  /*
   *  let a = new Data {
   *      member1: 100,
   *      member2: "hogehoge"
   *  };
   */
  char *tagname = Get_Step_Token()->value;
  Type *tag = new_type_unsolved(tagname);
  Vector *fields = NULL,
         *inits = NULL;

  if(skip(TKIND_Lbrace)) {
    fields = new_vector();
    inits = new_vector();

    for(int i = 0; !skip(TKIND_Rbrace); ++i) {
      if(i > 0) {
        expect(TKIND_Comma);
      }
      vec_push(fields, expr_var());
      expect(TKIND_Colon);

      vec_push(inits, expr());
    }
  }

  return (Ast *)node_struct_init(tag, fields, inits);
}

static Ast *statement() {
  if(Cur_Token_Is(TKIND_Lbrace)) {
    return make_block();
  }
  else if(skip(TKIND_For)) {
    return make_for();
  }
  else if(skip(TKIND_While)) {
    return make_while();
  }
  else if(skip(TKIND_If)) {
    return make_if(false);
  }
  else if(skip(TKIND_Return)) {
    return make_return();
  }
  else if(skip(TKIND_YIELD)) {
    return make_yield();
  }
  else if(skip(TKIND_Break)) {
    return make_break();
  }
  else if(skip(TKIND_Skip)) {
    return make_skip();
  }
  else if(skip(TKIND_Let)) {
    return var_decl(false);
  }
  else if(skip(TKIND_Const)) {
    return var_decl(true);
  }
  else if(skip(TKIND_ITERATOR)) {
    return func_def(true);
  }
  else if(skip(TKIND_Fn)) {
    return func_def(false);
  }
  else if(skip(TKIND_Object)) {
    return make_object();
  }
  else if(skip(TKIND_Use)) {
    return make_moduse();
  }
  else if(skip(TKIND_BreakPoint)) {
    return make_breakpoint();
  }
  else if(skip(TKIND_Assert)) {
    return make_assert();
  }
  else if(skip(TKIND_Typedef)) {
    make_typedef();
    return NULL;
  }
  else {
    return expr();
  }
}

static Vector *parser_main() {
  Vector *program = new_vector();

  while(!Cur_Token_Is(TKIND_End)) {
    Ast *st = statement();
    if(st) {
      vec_push(program, st);
    }

    if(ast_isexpr(st)) {
      expect(TKIND_Semicolon);
    }
  }

  return program;
}

static Vector *enter(Vector *tk) {
  vec_push(tokens_stack, tokens);
  vec_push(pos_stack, (void *)(intptr_t)pos);
  tokens = tk;
  pos = 0;

  Vector *result = parser_main();

  tokens = vec_pop(tokens_stack);
  pos = (intptr_t)vec_pop(pos_stack);
  nenter++;
  del_vector(tk);

  return result;
}

Vector *parser_run(Vector *_token) {
  tokens_stack = new_vector();
  pos_stack = new_vector();
  return enter(_token);
}

