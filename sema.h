#ifndef MAXC_SEMA_H
#define MAXC_SEMA_H

#include "ast.h"
#include "maxc.h"
#include "type.h"

class SemaAnalyzer {
  public:
    Ast_v &run(Ast_v &);

  private:
    Ast_v ret_ast;

    std::stack<NodeFunction *, std::vector<NodeFunction *>> fn_saver;

    Ast *visit(Ast *);
    Ast *visit_binary(Ast *);
    Ast *visit_assign(Ast *);
    Ast *visit_vardecl(Ast *);
    Ast *visit_return(Ast *);
    Ast *visit_load(Ast *);
    Ast *visit_fncall(Ast *);
    Ast *visit_funcdef(Ast *);
    Ast *visit_bltinfn_call(NodeFnCall *);

    Type *checktype(Type *, Type *);
};

#endif
