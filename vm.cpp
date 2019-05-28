#include "maxc.h"

#define Dispatch() do{ goto *codetable[(int)code[pc]]; } while(0)

#define List_Setitem(ob, index, item) (ob->elem[index] = item)
#define List_Getitem(ob, index) (ob->elem[index])

int VM::run(bytecode &code, Constant &ctable_) {
    ctable = ctable_;

    env = new VMEnv();
    env->cur = new vmenv_t();

    exec(code);

    return 0;
}

void VM::exec(bytecode &code) {
    static const void *codetable[] = {
        &&code_end,
        &&code_push,
        &&code_ipush,
        &&code_pushconst_1,
        &&code_pushconst_2,
        &&code_pushconst_3,
        &&code_pushtrue,
        &&code_pushfalse,
        &&code_pop,
        &&code_add,
        &&code_sub,
        &&code_mul,
        &&code_div,
        &&code_mod,
        &&code_logor,
        &&code_logand,
        &&code_eq,
        &&code_noteq,
        &&code_lt,
        &&code_lte,
        &&code_gt,
        &&code_gte,
        &&code_label,
        &&code_jmp,
        &&code_jmp_eq,
        &&code_jmp_noteq,
        &&code_inc,
        &&code_dec,
        &&code_print,
        &&code_println,
        &&code_format,
        &&code_typeof,
        &&code_load,
        &&code_store,
        &&code_istore,
        &&code_listset,
        &&code_subscr,
        &&code_subscr_store,
        &&code_stringset,
        &&code_tupleset,
        &&code_functionset,
        &&code_ret,
        &&code_call,
        &&code_callmethod,
        &&code_fnbegin,
        &&code_fnend,
    };

    goto *codetable[(int)code[pc]];

code_push:
    {
        ++pc;
        /*
        vmcode_t &c = code[pc];
        if(c.vtype == VALUE::Number) {
            int &_i = c.num;
            s.push(value_t(_i));
        }
        else if(c.vtype == VALUE::Char) {
            char &_c = c.ch;
            s.push(value_t(_c));
        }*/
        Dispatch();
    }
code_ipush:
    ++pc;
    stk.push(Object::alloc_intobject(
                Bytecode::read_int32(code, pc)
            ));

    Dispatch();
code_pushconst_1:
    ++pc;
    stk.push(Object::alloc_intobject(1));

    Dispatch();
code_pushconst_2:
    ++pc;
    stk.push(Object::alloc_intobject(2));

    Dispatch();
code_pushconst_3:
    ++pc;
    stk.push(Object::alloc_intobject(3));

    Dispatch();
code_pushtrue:
    ++pc;
    stk.push(Object::alloc_boolobject(true));

    Dispatch();
code_pushfalse:
    ++pc;
    stk.push(Object::alloc_boolobject(false));

    Dispatch();
code_pop:
    ++pc;
    stk.pop();

    Dispatch();
code_add:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_add(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_sub:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_sub(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_mul:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mul(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_div:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_div(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_mod:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mod(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_logor:
    {
        ++pc;
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logor(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_logand:
    {
        ++pc;
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logand(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_eq:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_eq(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_noteq:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_noteq(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_lt:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_lt(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_lte:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_lte(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_gt:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gt(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_gte:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gte(l, r));
        Object::decref(r); Object::decref(l);

        Dispatch();
    }
code_inc:
    {
        ++pc;
        auto u = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_inc(u));

        Dispatch();
    }
code_dec:
    {
        ++pc;
        auto u = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_dec(u));

        Dispatch();
    }
code_store:
    {
        ++pc;

        int key = Bytecode::read_int32(code, pc);

        MxcObject *ob = stk.top(); stk.pop();

        NodeVariable *var = ctable.table[key].var;
        if(var->isglobal) {
            gvmap[var] = ob;
        }
        else {
            env->cur->vmap[var] = ob;
        }

        Dispatch();
    }
code_istore:
    {
        /*
        vmcode_t &c = code[pc];
        MxcObject *ob = stk.top();
        if(c.var->isglobal)
            gvmap[c.var->vid] = ob;
        else
            env->cur->vmap[c.var->vid] = ob;
        stk.pop();
        */

        Dispatch();
    }
code_load:
    {
        ++pc;

        int key = Bytecode::read_int32(code, pc);

        NodeVariable *v = ctable.table[key].var;

        if(v->isglobal) {
            MxcObject *ob = gvmap.at(v);
            Object::incref(ob);
            stk.push(ob);
        }
        else {
            MxcObject *ob = env->cur->vmap.at(v);
            Object::incref(ob);
            stk.push(ob);
        }

        Dispatch();
    }
code_print:
    {
        ++pc;
        MxcObject *ob = stk.top();
        print(ob);
        stk.pop();
        Object::decref(ob);

        Dispatch();
    }
code_println:
    {
        ++pc;
        MxcObject *ob = stk.top();
        print(ob); puts("");
        stk.pop();
        Object::decref(ob);

        Dispatch();
    }
code_format:
    {
        ++pc;
        Dispatch();
    }
code_typeof:
    ++pc;
    Dispatch();
code_jmp:
    ++pc;

    pc = Bytecode::read_int32(code, pc);

    Dispatch();
code_jmp_eq:
    {
        ++pc;

        auto a = (BoolObject *)stk.top();

        if(a->boolean == true)
            pc = Bytecode::read_int32(code, pc);
        else
            pc += 4;

        Object::decref(a);
        stk.pop();

        Dispatch();
    }
code_jmp_noteq:
    {
        ++pc;

        auto a = (BoolObject *)stk.top();

        if(a->boolean == false)
            pc = Bytecode::read_int32(code, pc);
        else
            pc += 4;    //skip arg

        Object::decref(a);
        stk.pop();

        Dispatch();
    }
code_listset:
    {
        ++pc;
        /*
        auto ob = Object::alloc_listobject(code[pc].size);

        for(cnt = 0; cnt < code[pc].size; ++cnt) {
            List_Setitem(ob, cnt, stk.top()); stk.pop();
        }

        stk.push(ob);
        */
        Dispatch();
    }
code_subscr:
    {
        ++pc;
        auto ls = (ListObject *)stk.top(); stk.pop();
        auto idx = (IntObject *)stk.top(); stk.pop();
        auto ob = List_Getitem(ls, idx->inum32);
        Object::incref(ob);
        stk.push(ob);

        Dispatch();
    }
code_subscr_store:
    {
        ++pc;
        auto ob = (ListObject *)stk.top(); stk.pop();
        auto idx = (IntObject *)stk.top(); stk.pop();
        List_Setitem(ob, idx->inum32, stk.top());
        stk.pop();

        Dispatch();
    }
code_stringset:
    {
        ++pc;
        //stk.push(Object::alloc_stringobject(code[pc].str));

        Dispatch();
    }
code_tupleset:
    {
        ++pc;
        /*
        vmcode_t &c = code[pc];
        TupleObject tupob;
        for(lfcnt = 0; lfcnt < c.size; ++lfcnt) {
            tupob.tup.push_back(s.top()); s.pop();
        }
        s.push(value_t(tupob));*/
        Dispatch();
    }
code_functionset:
    {
        ++pc;
        //stk.push(Object::alloc_functionobject(code[pc].fnstart));

        Dispatch();
    }
code_fnbegin:
    {
        ++pc;
        /*
        for(;;) {
            ++pc;
            if(code[pc].type == OpCode::FNEND && code[pc].str == code[pc].str) break;
        }
        */

        Dispatch();
    }
code_call:
    {
        ++pc;
        //vmcode_t &c = code[pc];
        env->make();
        locs.push(pc);
        auto callee = (FunctionObject *)stk.top();
        fnstk.push(callee);
        pc = callee->start - 1; stk.pop();

        Dispatch();
    }
code_callmethod:
    {
        ++pc;
        /*
        vmcode_t &c = code[pc];
        switch(c.obmethod) {
            case Method::ListSize:
                {
                    cmlsob = s.top().listob; s.pop();
                    s.push(value_t((int)cmlsob.get_size()));
                } break;
            case Method::StringLength:
                {
                    cmstob = s.top().strob; s.pop();
                    s.push(value_t(cmstob.get_length()));
                } break;
            case Method::TupleAccess:
                {
                    cmtupob = s.top().tupleob; s.pop();
                    int &index = s.top().num; s.pop();
                    s.push(value_t(cmtupob.tup[index]));
                } break;
            default:
                error("unimplemented");
        }
        */
        Dispatch();
    }
code_ret:
    ++pc;
    pc = locs.top(); locs.pop();
    Object::decref(fnstk.top()); fnstk.pop();
    env->escape();

    Dispatch();
code_label:
code_fnend:
    ++pc;
    Dispatch();
code_end:
    return;
}

void VM::print(MxcObject *val) {
    switch(val->type) {
        case CTYPE::INT:
            printf("%d", ((IntObject *)val)->inum32);
            break;
        case CTYPE::BOOL:
            printf(((BoolObject *)val)->boolean ? "true" : "false");
            break;
        case CTYPE::CHAR:
            break;      //TODO
        case CTYPE::STRING:
            printf("%s", ((StringObject *)val)->str);
            break;
        case CTYPE::LIST: {
            printf("[ ");
            auto lob = (ListObject *)val;
            for(unsigned int i = 0; i < lob->allocated; ++i) {
                print(lob->elem[i]); putchar(' ');
            }
            printf("]");
            break;
        }
        default:
            runtime_err("unimpl");
    }
}

vmenv_t *VMEnv::make() {
    vmenv_t *e = new vmenv_t(cur);

    cur = e;
    return cur;
}

vmenv_t *VMEnv::escape() {
    vmenv_t *pe = cur->parent;
    for(auto itr = cur->vmap.begin(); itr != cur->vmap.end(); ++itr)
        Object::decref(itr->second);
    delete cur;
    cur = pe;
    return cur;
}
