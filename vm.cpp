#include "maxc.h"

#define Dispatch() do { goto *codetable[(int)code[pc]]; } while(0)

#define List_Setitem(ob, index, item) (ob->elem[index] = item)
#define List_Getitem(ob, index) (ob->elem[index])

#define READ_i32(code, pc)  \
    ((int32_t)(((uint8_t)code[pc + 3] << 24)    \
             + ((uint8_t)code[pc + 2] << 16)    \
             + ((uint8_t)code[pc + 1] <<  8)    \
             + ((uint8_t)code[pc + 0]     )))   \

#define INCREF(ob) (++ob->refcount)
#define DECREF(ob)  \
    do {                            \
        if(--ob->refcount == 0) {   \
            free(ob);               \
        }                           \
    } while(0)

int VM::run(bytecode &code) {
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
                READ_i32(code, pc)
            ));
    pc += 4;

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
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_sub:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_sub(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_mul:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mul(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_div:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_div(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_mod:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mod(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_logor:
    {
        ++pc;
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logor(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_logand:
    {
        ++pc;
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logand(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_eq:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_eq(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_noteq:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_noteq(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_lt:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_lt(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_lte:
    {
        ++pc;

        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();

        stk.push(Object::int_lte(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_gt:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gt(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_gte:
    {
        ++pc;
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gte(l, r));
        DECREF(r); DECREF(l);

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

        int key = READ_i32(code, pc);

        pc += 4;

        MxcObject *ob = stk.top(); stk.pop();

        NodeVariable *var = ctable->table[key].var;
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

        int key = READ_i32(code, pc);
        pc += 4;

        if(ctable->table[key].var->isglobal) {
            MxcObject *ob = gvmap.at(ctable->table[key].var);
            INCREF(ob);
            stk.push(ob);
        }
        else {
            MxcObject *ob = env->cur->vmap.at(ctable->table[key].var);
            INCREF(ob);
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
        DECREF(ob);

        Dispatch();
    }
code_println:
    {
        ++pc;
        MxcObject *ob = stk.top();
        print(ob); puts("");
        stk.pop();
        DECREF(ob);

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

    pc = READ_i32(code, pc);

    Dispatch();
code_jmp_eq:
    {
        ++pc;

        auto a = (BoolObject *)stk.top();

        if(a->boolean == true)
            pc = READ_i32(code, pc);
        else
            pc += 4;

        DECREF(a);
        stk.pop();

        Dispatch();
    }
code_jmp_noteq:
    {
        ++pc;

        auto a = (BoolObject *)stk.top();

        if(a->boolean == false)
            pc = READ_i32(code, pc);
        else
            pc += 4;    //skip arg

        DECREF(a);
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
        INCREF(ob);
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
    DECREF(fnstk.top()); fnstk.pop();
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
        DECREF(itr->second);

    delete cur;

    cur = pe;

    return cur;
}
