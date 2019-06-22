#include "maxc.h"

#define Dispatch() do { goto *codetable[(uint8_t)frame->code[frame->pc]]; } while(0)

#define List_Setitem(ob, index, item) (ob->elem[index] = item)
#define List_Getitem(ob, index) (ob->elem[index])

#define READ_i32(code, pc)  \
    ((int32_t)(((uint8_t)code[pc + 3] << 24)    \
             + ((uint8_t)code[pc + 2] << 16)    \
             + ((uint8_t)code[pc + 1] <<  8)    \
             + ((uint8_t)code[pc + 0]     )))   \

//reference counter
#define INCREF(ob) (++ob->refcount)
#define DECREF(ob)  \
    do {                            \
        if(--ob->refcount == 0) {   \
            free(ob);               \
        }                           \
    } while(0)

//stack
#define Push(ob) (*(stackptr++) = (ob))
#define Pop() (*(--stackptr))
#define Top() (stackptr[-1])
#define SetTop(ob) (stackptr[-1] = ob)


int VM::run(bytecode &code) {
    frame = new Frame(code);    //global frame

    stackptr = (MxcObject **)malloc(sizeof(MxcObject *) * 1000);
    printf("%p\n", stackptr);

    int ret = exec();

    printf("%p\n", stackptr);

    return ret;
}

int VM::exec() {
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
        &&code_load_global,
        &&code_load_local,
        &&code_store_global,
        &&code_store_local,
        &&code_listset,
        &&code_subscr,
        &&code_subscr_store,
        &&code_stringset,
        &&code_tupleset,
        &&code_functionset,
        &&code_ret,
        &&code_call,
        &&code_callmethod,
    };

    goto *codetable[(uint8_t)frame->code[frame->pc]];

code_push:
    {
        ++frame->pc;
        /*
        vmcode_t &c = code[frame->pc];
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
    ++frame->pc;
    Push(Object::alloc_intobject(
                READ_i32(frame->code, frame->pc)
            ));
    frame->pc += 4;

    Dispatch();
code_pushconst_1:
    ++frame->pc;
    Push(Object::alloc_intobject(1));

    Dispatch();
code_pushconst_2:
    ++frame->pc;
    Push(Object::alloc_intobject(2));

    Dispatch();
code_pushconst_3:
    ++frame->pc;
    Push(Object::alloc_intobject(3));

    Dispatch();
code_pushtrue:
    ++frame->pc;
    Push(Object::alloc_boolobject(true));

    Dispatch();
code_pushfalse:
    ++frame->pc;
    Push(Object::alloc_boolobject(false));

    Dispatch();
code_pop:
    ++frame->pc;
    Pop();

    Dispatch();
code_add:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_add(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_sub:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_sub(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_mul:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_mul(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_div:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_div(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_mod:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_mod(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_logor:
    {
        ++frame->pc;

        auto r = (BoolObject *)Pop();
        auto l = (BoolObject *)Pop();

        Push(Object::bool_logor(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_logand:
    {
        ++frame->pc;

        auto r = (BoolObject *)Pop();
        auto l = (BoolObject *)Pop();

        Push(Object::bool_logand(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_eq:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_eq(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_noteq:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_noteq(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_lt:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_lt(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_lte:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_lte(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_gt:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_gt(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_gte:
    {
        ++frame->pc;

        auto r = (IntObject *)Pop();
        auto l = (IntObject *)Pop();

        Push(Object::int_gte(l, r));
        DECREF(r); DECREF(l);

        Dispatch();
    }
code_inc:
    {
        ++frame->pc;

        auto u = (IntObject *)Pop();

        Push(Object::int_inc(u));

        Dispatch();
    }
code_dec:
    {
        ++frame->pc;

        auto u = (IntObject *)Pop();

        Push(Object::int_dec(u));

        Dispatch();
    }
code_store_global:
    {
        ++frame->pc;

        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        NodeVariable *var = ctable->table[key].var;
        gvmap[var] = Pop();

        Dispatch();
    }
code_store_local:
    {
        ++frame->pc;

        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        NodeVariable *var = ctable->table[key].var;
        frame->lvars[var] = Pop();

        Dispatch();
    }
code_load_global:
    {
        ++frame->pc;

        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        MxcObject *ob = gvmap[ctable->table[key].var];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
code_load_local:
    {
        ++frame->pc;

        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        MxcObject *ob = frame->lvars[ctable->table[key].var];
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
code_print:
    {
        ++frame->pc;

        MxcObject *ob = Pop();
        print(ob);
        DECREF(ob);

        Dispatch();
    }
code_println:
    {
        ++frame->pc;

        MxcObject *ob = Pop();
        print(ob); puts("");
        DECREF(ob);

        Dispatch();
    }
code_format:
code_typeof:
    ++frame->pc;
    Dispatch();
code_jmp:
    ++frame->pc;

    frame->pc = READ_i32(frame->code, frame->pc);

    Dispatch();
code_jmp_eq:
    {
        ++frame->pc;

        auto a = (BoolObject *)Pop();

        if(a->boolean == true)
            frame->pc = READ_i32(frame->code, frame->pc);
        else
            frame->pc += 4;

        DECREF(a);

        Dispatch();
    }
code_jmp_noteq:
    {
        ++frame->pc;

        auto a = (BoolObject *)Pop();

        if(a->boolean == false)
            frame->pc = READ_i32(frame->code, frame->pc);
        else
            frame->pc += 4;    //skip arg

        DECREF(a);

        Dispatch();
    }
code_listset:
    {
        ++frame->pc;
        /*
        auto ob = Object::alloc_listobject(code[frame->pc].size);

        for(cnt = 0; cnt < code[frame->pc].size; ++cnt) {
            List_Setitem(ob, cnt, stk.top()); Pop();
        }

        Push(ob);
        */
        Dispatch();
    }
code_subscr:
    {
        ++frame->pc;
        auto ls = (ListObject *)Pop();
        auto idx = (IntObject *)Pop();
        auto ob = List_Getitem(ls, idx->inum32);
        INCREF(ob);
        Push(ob);

        Dispatch();
    }
code_subscr_store:
    {
        ++frame->pc;
        auto ob = (ListObject *)Pop();
        auto idx = (IntObject *)Pop();
        List_Setitem(ob, idx->inum32, Pop());

        Dispatch();
    }
code_stringset:
    {
        ++frame->pc;
        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(Object::alloc_stringobject(
                    ctable->table[key].str
                ));

        Dispatch();
    }
code_tupleset:
    {
        ++frame->pc;
        /*
        vmcode_t &c = code[frame->pc];
        TupleObject tupob;
        for(lfcnt = 0; lfcnt < c.size; ++lfcnt) {
            tupob.tup.push_back(s.top()); s.pop();
        }
        s.push(value_t(tupob));*/
        Dispatch();
    }
code_functionset:
    {
        ++frame->pc;
        //Push(Object::alloc_functionobject(code[frame->pc].fnstart));
        int key = READ_i32(frame->code, frame->pc);
        frame->pc += 4;

        Push(Object::alloc_functionobject(ctable->table[key].func));

        Dispatch();
    }
code_call:
    {
        ++frame->pc;

        framestack.push(frame);

        auto callee = (FunctionObject *)Pop();

        frame = new Frame(callee->func);

        exec();

        frame = framestack.top();
        framestack.pop();
        /*vmcode_t &c = code[frame->pc];
        env->make();
        locs.push(frame->pc);
        auto callee = (FunctionObject *)Pop();
        fnstk.push(callee);
        frame->pc = callee->start - 1;*/

        Dispatch();
    }
code_callmethod:
    {
        ++frame->pc;
        /*
        vmcode_t &c = code[frame->pc];
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
    ++frame->pc;

    for(auto itr = frame->lvars.begin(); itr != frame->lvars.end(); ++itr) {
        DECREF(itr->second);
    }

    delete frame;

    /*
    frame->pc = locs.top(); locs.pop();
    DECREF(fnstk.top()); fnstk.pop();
    env->escape();*/

    return 0;
code_label:
    ++frame->pc;
    Dispatch();
code_end:
    return 0;
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
            printf("unimpl: %d", (int)val->type);
    }
}
/*
vmenv_t *VMEnv::make() {
    vmenv_t *e = new vmenv_t(cur);

    cur = e;
    return cur;
}

vmenv_t *VMEnv::escape() {
    vmenv_t *pe = cur->parent;


    delete cur;

    cur = pe;

    return cur;
} */
