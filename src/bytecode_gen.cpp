#include "bytecode_gen.h"
#include "bytecode.h"
#include "error.h"
#include "maxc.h"

void BytecodeGenerator::compile(Ast_v &asts, bytecode &iseq) {
    for(Ast *ast : asts)
        gen(ast, iseq, false);

    Bytecode::push_0arg(iseq, OpCode::END);
}

void BytecodeGenerator::gen(Ast *ast, bytecode &iseq, bool use_ret) {
    if(ast == nullptr) {
        return;
    }

    switch(ast->get_nd_type()) {
    case NDTYPE::NUM:
        emit_num(ast, iseq, use_ret);
        break;
    case NDTYPE::BOOL:
        emit_bool(ast, iseq, use_ret);
        break;
    case NDTYPE::CHAR:
        emit_char(ast, iseq, use_ret);
        break;
    case NDTYPE::STRING:
        emit_string(ast, iseq, use_ret);
        break;
    case NDTYPE::STRUCT:
        break;
    case NDTYPE::LIST:
        emit_list(ast, iseq);
        break;
    case NDTYPE::SUBSCR:
        emit_listaccess(ast, iseq);
        break;
    case NDTYPE::TUPLE:
        emit_tuple(ast, iseq);
        break;
    case NDTYPE::BINARY:
        emit_binop(ast, iseq, use_ret);
        break;
    case NDTYPE::MEMBER:
        emit_member(ast, iseq, use_ret);
        break;
    case NDTYPE::UNARY:
        emit_unaop(ast, iseq, use_ret);
        break;
    case NDTYPE::TERNARY:
        emit_ternop(ast, iseq);
        break;
    case NDTYPE::ASSIGNMENT:
        emit_assign(ast, iseq);
        break;
    case NDTYPE::IF:
    case NDTYPE::EXPRIF:
        emit_if(ast, iseq);
        break;
    case NDTYPE::FOR:
        emit_for(ast, iseq);
        break;
    case NDTYPE::WHILE:
        emit_while(ast, iseq);
        break;
    case NDTYPE::BLOCK:
        emit_block(ast, iseq);
        break;
    case NDTYPE::RETURN:
        emit_return(ast, iseq);
        break;
    case NDTYPE::VARIABLE:
        emit_load(ast, iseq, use_ret);
        break;
    case NDTYPE::FUNCCALL:
        emit_func_call(ast, iseq, use_ret);
        break;
    case NDTYPE::FUNCDEF:
        emit_func_def(ast, iseq);
        break;
    case NDTYPE::VARDECL:
        emit_vardecl(ast, iseq);
        break;
    default:
        error("??? in gen");
    }
}

void BytecodeGenerator::emit_num(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeNumber *n = (NodeNumber *)ast;

    if(n->ctype->isfloat()) {
        int key = ltable.push_float(n->fnumber);

        Bytecode::push_fpush(iseq, key);
    }
    else {
        if(n->number == 0) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_0);
        }
        else if(n->number == 1) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_1);
        }
        else if(n->number == 2) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_2);
        }
        else if(n->number == 3) {
            Bytecode::push_0arg(iseq, OpCode::PUSHCONST_3);
        }
        else
            Bytecode::push_ipush(iseq, n->number);
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_bool(Ast *ast, bytecode &iseq, bool use_ret) {
    auto b = (NodeBool *)ast;

    if(b->boolean)
        Bytecode::push_0arg(iseq, OpCode::PUSHTRUE);
    else
        Bytecode::push_0arg(iseq, OpCode::PUSHFALSE);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_char(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeChar *c = (NodeChar *)ast;

    // vcpush(OpCode::PUSH, (char)c->ch);
}

void BytecodeGenerator::emit_string(Ast *ast, bytecode &iseq, bool use_ret) {
    int key = ltable.push_str(((NodeString *)ast)->string);

    Bytecode::push_strset(iseq, key);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_list(Ast *ast, bytecode &iseq) {
    auto *l = (NodeList *)ast;

    for(int i = (int)l->nsize - 1; i >= 0; i--)
        gen(l->elem[i], iseq, true);
    // vcpush(OpCode::LISTSET, l->nsize);
}

void BytecodeGenerator::emit_listaccess(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;

    if(l->istuple) {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        // vcpush(OpCode::CALLMethod, Method::TupleAccess);
    }
    else {
        gen(l->index, iseq, true);
        gen(l->ls, iseq, false);
        Bytecode::push_0arg(iseq, OpCode::SUBSCR);
    }
}

void BytecodeGenerator::emit_tuple(Ast *ast, bytecode &iseq) {
    auto t = (NodeTuple *)ast;

    for(int i = (int)t->nsize - 1; i >= 0; i--)
        gen(t->exprs[i], iseq, true);

    // vcpush(OpCode::TUPLESET, t->nsize);
}

void BytecodeGenerator::emit_binop(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeBinop *b = (NodeBinop *)ast;

    gen(b->left, iseq, true);
    gen(b->right, iseq, true);

    if(!b->left->ctype->isfloat()) {
        if(b->op == "+") {
            Bytecode::push_0arg(iseq, OpCode::ADD);
        }
        else if(b->op == "-") {
            Bytecode::push_0arg(iseq, OpCode::SUB);
        }
        else if(b->op == "*") {
            Bytecode::push_0arg(iseq, OpCode::MUL);
        }
        else if(b->op == "/") {
            Bytecode::push_0arg(iseq, OpCode::DIV);
        }
        else if(b->op == "%") {
            Bytecode::push_0arg(iseq, OpCode::MOD);
        }
        else if(b->op == "==") {
            Bytecode::push_0arg(iseq, OpCode::EQ);
        }
        else if(b->op == "!=") {
            Bytecode::push_0arg(iseq, OpCode::NOTEQ);
        }
        else if(b->op == "||") {
            Bytecode::push_0arg(iseq, OpCode::LOGOR);
        }
        else if(b->op == "&&") {
            Bytecode::push_0arg(iseq, OpCode::LOGAND);
        }
        else if(b->op == "<") {
            Bytecode::push_0arg(iseq, OpCode::LT);
        }
        else if(b->op == "<=") {
            Bytecode::push_0arg(iseq, OpCode::LTE);
        }
        else if(b->op == ">") {
            Bytecode::push_0arg(iseq, OpCode::GT);
        }
        else if(b->op == ">=") {
            Bytecode::push_0arg(iseq, OpCode::GTE);
        }
    }
    else {
        if(b->op == "+") {
            Bytecode::push_0arg(iseq, OpCode::FADD);
        }
        else if(b->op == "-") {
            Bytecode::push_0arg(iseq, OpCode::FSUB);
        }
        else if(b->op == "*") {
            Bytecode::push_0arg(iseq, OpCode::FMUL);
        }
        else if(b->op == "/") {
            Bytecode::push_0arg(iseq, OpCode::FDIV);
        }
        else if(b->op == "%") {
            Bytecode::push_0arg(iseq, OpCode::FMOD);
        }
        else if(b->op == "==") {
            Bytecode::push_0arg(iseq, OpCode::FEQ);
        }
        else if(b->op == "!=") {
            Bytecode::push_0arg(iseq, OpCode::FNOTEQ);
        }
        else if(b->op == "||") {
            Bytecode::push_0arg(iseq, OpCode::FLOGOR);
        }
        else if(b->op == "&&") {
            Bytecode::push_0arg(iseq, OpCode::FLOGAND);
        }
        else if(b->op == "<") {
            Bytecode::push_0arg(iseq, OpCode::FLT);
        }
        else if(b->op == "<=") {
            Bytecode::push_0arg(iseq, OpCode::FLTE);
        }
        else if(b->op == ">") {
            Bytecode::push_0arg(iseq, OpCode::FGT);
        }
        else if(b->op == ">=") {
            Bytecode::push_0arg(iseq, OpCode::FGTE);
        }
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_member(Ast *ast, bytecode &iseq, bool use_ret) {
    auto m = (NodeMember *)ast;

    gen(m->left, iseq, true);

    gen(m->right, iseq, true);
}

void BytecodeGenerator::emit_ternop(Ast *ast, bytecode &iseq) {
    auto t = (NodeTernop *)ast;

    gen(t->cond, iseq, true);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(t->then, iseq, true);

    size_t then_epos = iseq.size();
    Bytecode::push_jmp(iseq, 0); // goto if statement end

    size_t else_spos = iseq.size();
    Bytecode::replace_int32(cpos, iseq, else_spos);

    gen(t->els, iseq, true);

    size_t epos = iseq.size();
    Bytecode::replace_int32(then_epos, iseq, epos);
}

void BytecodeGenerator::emit_unaop(Ast *ast, bytecode &iseq, bool use_ret) {
    auto u = (NodeUnaop *)ast;

    gen(u->expr, iseq, true);

    /*
    if(u->op == "!") {
        puts("\tcmp $0, %rax");
        puts("\tsete %al");
        puts("\tmovzb %al, %rax");
        return;
    }
    */
    if(u->op == "++") {
        Bytecode::push_0arg(iseq, OpCode::INC);
    }
    else if(u->op == "--") {
        Bytecode::push_0arg(iseq, OpCode::DEC);
    }

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_assign(Ast *ast, bytecode &iseq) {
    // debug("called assign\n");
    auto a = (NodeAssignment *)ast;

    gen(a->src, iseq, true);

    if(a->dst->get_nd_type() == NDTYPE::SUBSCR)
        emit_listaccess_store(a->dst, iseq);
    else if(a->dst->get_nd_type() == NDTYPE::MEMBER)
        emit_member_store(a->dst, iseq);
    else
        emit_store(a->dst, iseq);
}

void BytecodeGenerator::emit_store(Ast *ast, bytecode &iseq) {
    NodeVariable *v = (NodeVariable *)ast;

    Bytecode::push_store(iseq, v->vid, v->isglobal);
}

void BytecodeGenerator::emit_member_store(Ast *ast, bytecode &iseq) {
    auto m = (NodeMember *)ast;

    gen(m->left, iseq, true);
}

void BytecodeGenerator::emit_listaccess_store(Ast *ast, bytecode &iseq) {
    auto l = (NodeSubscript *)ast;

    gen(l->index, iseq, true);
    gen(l->ls, iseq, false);

    Bytecode::push_0arg(iseq, OpCode::SUBSCR_STORE);
}

void BytecodeGenerator::emit_func_def(Ast *ast, bytecode &iseq) {
    auto f = (NodeFunction *)ast;

    bytecode fn_iseq;

    for(int n = f->finfo.args.get().size() - 1; n >= 0; n--) {
        auto a = f->finfo.args.get()[n];
        emit_store(a, fn_iseq);
    }

    if(f->block->get_nd_type() == NDTYPE::BLOCK) {
        NodeBlock *b = (NodeBlock *)f->block;
        for(size_t i = 0; i < b->cont.size(); i++) {
            gen(b->cont[i],
                fn_iseq,
                i == b->cont.size() - 1 ? true : false); // last expression
        }
    }
    else {
        gen(f->block, fn_iseq, true);
    }

    Bytecode::push_0arg(fn_iseq, OpCode::RET);

    userfunction fn_object;
    new_userfunction(fn_object, fn_iseq, f->lvars);

    int key = ltable.push_userfunc(fn_object);

    Bytecode::push_functionset(iseq, key);

    /*
    lmap[f->name] = nline;
    vcpush(OpCode::FNBEGIN, f->name);

    vcpush(OpCode::FNEND, f->name);
    */

    emit_store(f->fnvar, iseq);
}

void BytecodeGenerator::emit_if(Ast *ast, bytecode &iseq) {
    auto i = (NodeIf *)ast;

    gen(i->cond, iseq, true);

    size_t cpos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(i->then_s, iseq, i->isexpr);

    if(i->else_s) {
        size_t then_epos = iseq.size();
        Bytecode::push_jmp(iseq, 0); // goto if statement end

        size_t else_spos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, else_spos);

        gen(i->else_s, iseq, i->isexpr);

        size_t epos = iseq.size();
        Bytecode::replace_int32(then_epos, iseq, epos);
    }
    else {
        size_t pos = iseq.size();
        Bytecode::replace_int32(cpos, iseq, pos);
    }
}

void BytecodeGenerator::emit_for(Ast *ast, bytecode &iseq) {
    /*
    auto f = (NodeFor *)ast;

    if(f->init)
        gen(f->init, iseq, true);
    char *begin = get_label();
    char *end = get_label();
    lmap[begin] = nline;
    vcpush(OpCode::LABEL, begin);
    if(f->cond) {
        gen(f->cond, iseq, true);
        vcpush(OpCode::JMP_NOTEQ, end);
    }
    gen(f->body, iseq, false);
    if(f->reinit)
        gen(f->reinit, iseq, true);
    vcpush(OpCode::JMP, begin);
    lmap[end] = nline;
    vcpush(OpCode::LABEL, end);*/
}

void BytecodeGenerator::emit_while(Ast *ast, bytecode &iseq) {
    auto w = (NodeWhile *)ast;

    size_t begin = iseq.size();

    gen(w->cond, iseq, true);

    size_t pos = iseq.size();
    Bytecode::push_jmpneq(iseq, 0);

    gen(w->body, iseq, false);

    Bytecode::push_jmp(iseq, begin);

    size_t end = iseq.size();
    Bytecode::replace_int32(pos, iseq, end);
}

void BytecodeGenerator::emit_return(Ast *ast, bytecode &iseq) {
    gen(((NodeReturn *)ast)->cont, iseq, true);

    Bytecode::push_0arg(iseq, OpCode::RET);
}

void BytecodeGenerator::emit_func_call(Ast *ast, bytecode &iseq, bool use_ret) {
    auto f = (NodeFnCall *)ast;

    if(((NodeVariable *)f->func)->finfo.isbuiltin) {
        return emit_bltinfunc_call(f, iseq, use_ret);
    }

    for(auto a : f->args)
        gen(a, iseq, true);

    gen(f->func, iseq, true);

    Bytecode::push_0arg(iseq, OpCode::CALL);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_bltinfunc_call(NodeFnCall *f,
                                            bytecode &iseq,
                                            bool use_ret) {

    NodeVariable *fn = (NodeVariable *)f->func;

    if(fn->finfo.fnkind == BltinFnKind::Print) {
        return emit_bltinfncall_print(f, iseq, false);
    }

    if(fn->finfo.fnkind == BltinFnKind::Println) {
        return emit_bltinfncall_println(f, iseq, false);
    }

    for(auto &a : f->args)
        gen(a, iseq, true);

    BltinFnKind callfn = fn->finfo.fnkind;

    switch(fn->finfo.fnkind) {
    case BltinFnKind::StringSize:
        callfn = BltinFnKind::StringSize;
        break;
    case BltinFnKind::StringIsempty:
        callfn = BltinFnKind::StringIsempty;
        break;
    case BltinFnKind::IntToFloat:
        callfn = BltinFnKind::IntToFloat;
        break;
    case BltinFnKind::ObjectId:
        callfn = BltinFnKind::ObjectId;
        break;
    default:
        error("unimplemented: No function in bytecode_gen.cpp");
    }

    Bytecode::push_bltinfn_set(iseq, callfn);

    Bytecode::push_bltinfn_call(iseq, f->args.size());

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}

void BytecodeGenerator::emit_bltinfncall_println(NodeFnCall *f,
                                                 bytecode &iseq,
                                                 bool use_ret) {
    NodeVariable *fn = (NodeVariable *)f->func;

    for(size_t i = 0; i < f->args.size(); ++i) {
        BltinFnKind callfn = fn->finfo.fnkind;

        gen(f->args[i], iseq, true);

        switch(f->args[i]->ctype->type.type) {
        case CTYPE::INT:
            callfn = i != f->args.size() - 1 ? BltinFnKind::PrintInt
                                             : BltinFnKind::PrintlnInt;
            break;
        case CTYPE::DOUBLE:
            callfn = i != f->args.size() - 1 ? BltinFnKind::PrintFloat
                                             : BltinFnKind::PrintlnFloat;
            break;
        case CTYPE::BOOL:
            callfn = i != f->args.size() - 1 ? BltinFnKind::PrintBool
                                             : BltinFnKind::PrintlnBool;
            break;
        case CTYPE::CHAR:
            callfn = i != f->args.size() - 1 ? BltinFnKind::PrintChar
                                             : BltinFnKind::PrintlnChar;
            break;
        case CTYPE::STRING:
            callfn = i != f->args.size() - 1 ? BltinFnKind::PrintString
                                             : BltinFnKind::PrintlnString;
            break;
        default:
            error("unimplemented: Print");
        }

        Bytecode::push_bltinfn_set(iseq, callfn);

        Bytecode::push_bltinfn_call(iseq, 1);

        if(!use_ret)
            Bytecode::push_0arg(iseq, OpCode::POP);
    }
}

void BytecodeGenerator::emit_bltinfncall_print(NodeFnCall *f,
                                               bytecode &iseq,
                                               bool use_ret) {
    NodeVariable *fn = (NodeVariable *)f->func;

    for(size_t i = 0; i < f->args.size(); ++i) {
        BltinFnKind callfn = fn->finfo.fnkind;

        gen(f->args[i], iseq, true);

        switch(f->args[i]->ctype->type.type) {
        case CTYPE::INT:
            callfn = BltinFnKind::PrintInt;
            break;
        case CTYPE::DOUBLE:
            callfn = BltinFnKind::PrintFloat;
            break;
        case CTYPE::BOOL:
            callfn = BltinFnKind::PrintBool;
            break;
        case CTYPE::CHAR:
            callfn = BltinFnKind::PrintChar;
            break;
        case CTYPE::STRING:
            callfn = BltinFnKind::PrintString;
            break;
        default:
            error("unimplemented: Print");
        }

        Bytecode::push_bltinfn_set(iseq, callfn);

        Bytecode::push_bltinfn_call(iseq, 1);

        if(!use_ret)
            Bytecode::push_0arg(iseq, OpCode::POP);
    }
}

void BytecodeGenerator::emit_block(Ast *ast, bytecode &iseq) {
    auto b = (NodeBlock *)ast;

    for(Ast *a : b->cont)
        gen(a, iseq, false);
}

void BytecodeGenerator::emit_vardecl(Ast *ast, bytecode &iseq) {
    NodeVardecl *v = (NodeVardecl *)ast;

    if(v->init != nullptr) {
        gen(v->init, iseq, true);

        emit_store(v->var, iseq);
    }
}

void BytecodeGenerator::emit_load(Ast *ast, bytecode &iseq, bool use_ret) {
    NodeVariable *v = (NodeVariable *)ast;

    Bytecode::push_load(iseq, v->vid, v->isglobal);

    if(!use_ret)
        Bytecode::push_0arg(iseq, OpCode::POP);
}
