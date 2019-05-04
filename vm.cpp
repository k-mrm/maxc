#include "maxc.h"

#define Jmpcode() do{ ++pc; goto *codetable[(int)code[pc].type]; } while(0)

#define List_Setitem(ob, index, item) (ob->elem[index] = item)
#define List_Getitem(ob, index) (ob->elem[index])

int VM::run(std::vector<vmcode_t> &code, std::map<const char *, int> &lmap) {
    if(!lmap.empty()) labelmap = lmap;
    env = new VMEnv();
    env->cur = new vmenv_t();
    exec(code);
    return 0;
}

void VM::exec(std::vector<vmcode_t> &code) {
    static const void *codetable[] = {
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
        &&code_end,
    };

    goto *codetable[(int)code[pc].type];

code_push:
    {
        vmcode_t &c = code[pc];
        if(c.vtype == VALUE::Number) {
            int &_i = c.num;
            s.push(value_t(_i));
        }
        else if(c.vtype == VALUE::Char) {
            char &_c = c.ch;
            s.push(value_t(_c));
        }
        Jmpcode();
    }
code_ipush:
    stk.push(Object::alloc_intobject(code[pc].num));
    Jmpcode();
code_pushconst_1:
    stk.push(Object::alloc_intobject(1));
    Jmpcode();
code_pushconst_2:
    stk.push(Object::alloc_intobject(2));
    Jmpcode();
code_pushconst_3:
    stk.push(Object::alloc_intobject(3));
    Jmpcode();
code_pushtrue:
    stk.push(Object::alloc_boolobject(true));
    Jmpcode();
code_pushfalse:
    stk.push(Object::alloc_boolobject(false));
    Jmpcode();
code_pop:
    stk.pop();
    Jmpcode();
code_add:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_add(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_sub:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_sub(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_mul:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mul(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_div:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_div(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_mod:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_mod(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_logor:
    {
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logor(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_logand:
    {
        auto r = (BoolObject *)stk.top(); stk.pop();
        auto l = (BoolObject *)stk.top(); stk.pop();
        stk.push(Object::bool_logand(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_eq:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_eq(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_noteq:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_noteq(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_lt:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_lt(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_lte:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_lte(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_gt:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gt(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_gte:
    {
        auto r = (IntObject *)stk.top(); stk.pop();
        auto l = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_gte(l, r));
        Object::decref(r); Object::decref(l);
        Jmpcode();
    }
code_inc:
    {
        auto u = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_inc(u));
        Jmpcode();
    }
code_dec:
    {
        auto u = (IntObject *)stk.top(); stk.pop();
        stk.push(Object::int_dec(u));
        Jmpcode();
    }
code_store:
    {
        vmcode_t &c = code[pc];

        /*
        switch(c.var->var->ctype->get().type) {
            case CTYPE::INT:
                valstr = value_t(s.top().num); break;
            case CTYPE::CHAR:
                valstr = value_t(s.top().ch); break;
            case CTYPE::STRING:
                valstr = value_t(s.top().strob); break;
            case CTYPE::LIST:
                valstr = value_t(s.top().listob); break;
            default:
                runtime_err("unimplemented");
        }*/
        MxcObject *ob = stk.top();
        if(c.var->isglobal) {
            gvmap[c.var->vid] = ob;
        }
        else {
            env->cur->vmap[c.var->vid] = ob;
        }
        stk.pop();
        Jmpcode();
    }
code_istore:
    {
        vmcode_t &c = code[pc];
        MxcObject *ob = stk.top();
        if(c.var->isglobal)
            gvmap[c.var->vid] = ob;
        else
            env->cur->vmap[c.var->vid] = ob;
        stk.pop();
        Jmpcode();
    }
code_load:
    {
        vmcode_t &c = code[pc];
        if(c.var->isglobal) {
            MxcObject *ob = gvmap.at(c.var->vid);
            Object::incref(ob);
            stk.push(ob);
        }
        else {
            //std::cout << c.var->var->vid << "\n";
            MxcObject *ob = env->cur->vmap.at(c.var->vid);
            Object::incref(ob);
            stk.push(ob);
        }
        Jmpcode();
    }
code_print:
    {
        MxcObject *ob = stk.top();
        print(ob);
        stk.pop();
        Object::decref(ob);
        Jmpcode();
    }
code_println:
    {
        MxcObject *ob = stk.top();
        print(ob); puts("");
        stk.pop();
        Object::decref(ob);
        Jmpcode();
    }
code_format:
    {
        Jmpcode();
    }
code_typeof:
    Jmpcode();
code_jmp:
    pc = labelmap[code[pc].str];
    Jmpcode();
code_jmp_eq:
    {
        auto a = (BoolObject *)stk.top();
        if(a->boolean == true)
            pc = labelmap[code[pc].str];
        Object::decref(a);
        stk.pop();
        Jmpcode();
    }
code_jmp_noteq:
    {
        auto a = (BoolObject *)stk.top();
        if(a->boolean == false)
            pc = labelmap[code[pc].str];
        Object::decref(a);
        stk.pop();
        Jmpcode();
    }
code_listset:
    {
        auto ob = Object::alloc_listobject(code[pc].size);

        for(cnt = 0; cnt < code[pc].size; ++cnt) {
            List_Setitem(ob, cnt, stk.top()); stk.pop();
        }

        stk.push(ob);
        Jmpcode();
    }
code_subscr:
    {
        auto ls = (ListObject *)stk.top(); stk.pop();
        auto idx = (IntObject *)stk.top(); stk.pop();
        auto ob = List_Getitem(ls, idx->inum32);
        Object::incref(ob);
        stk.push(ob);
        Jmpcode();
    }
code_subscr_store:
    {
        auto ob = (ListObject *)stk.top(); stk.pop();
        auto idx = (IntObject *)stk.top(); stk.pop();
        List_Setitem(ob, idx->inum32, stk.top());
        stk.pop();
        Jmpcode();
    }
code_stringset:
    {
        stk.push(Object::alloc_stringobject(code[pc].str));
        Jmpcode();
    }
code_tupleset:
    {
        /*
        vmcode_t &c = code[pc];
        TupleObject tupob;
        for(lfcnt = 0; lfcnt < c.size; ++lfcnt) {
            tupob.tup.push_back(s.top()); s.pop();
        }
        s.push(value_t(tupob));*/
        Jmpcode();
    }
code_functionset:
    {
        stk.push(Object::alloc_functionobject(code[pc].fnstart));
        Jmpcode();
    }
code_fnbegin:
    {
        for(;;) {
            ++pc;
            if(code[pc].type == OPCODE::FNEND && code[pc].str == code[pc].str) break;
        }
        Jmpcode();
    }
code_call:
    {
        //vmcode_t &c = code[pc];
        env->make();
        locs.push(pc);
        auto f = (FunctionObject *)stk.top();
        fnstk.push(f);
        pc = f->start - 1; stk.pop();
        Jmpcode();
    }
code_callmethod:
    {
        vmcode_t &c = code[pc];
        switch(c.obmethod) {
            /*
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
                } break;*/
            default:
                error("unimplemented");
        }
        Jmpcode();
    }
code_ret:
    pc = locs.top(); locs.pop();
    Object::decref(fnstk.top()); fnstk.pop();
    env->escape();
    Jmpcode();
code_label:
code_fnend:
    Jmpcode();
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
