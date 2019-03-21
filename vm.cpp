#include "maxc.h"

int VM::run(std::vector<vmcode_t> &code, std::map<std::string, int> &lmap) {
    if(!lmap.empty())
        labelmap = lmap;
    if(code.empty())
        return 1;
    env.cur = new vmenv_t();
    exec(code);
    return 0;
}

void VM::exec(std::vector<vmcode_t> &code) {
    static const void *codetable[] = {
        &&code_push,
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
        &&code_listset,
        &&code_ret,
        &&code_call,
        &&code_callmethod,
        &&code_fnbegin,
        &&code_fnend,
        &&code_end,
    };
    pc = 0;
    vmcode_t c = vmcode_t();
    value_t r, l, u;    //binary, unary
    value_t valstr;     //variable store
    std::string _format; int fpos; std::string bs; std::string ftop; //format
    std::string tyname;
    ListObject lsob; size_t lfcnt; std::vector<value_t> le;    //list
    ListObject cmlsob;

    goto *codetable[(int)code[pc].type];

code_push:
    c = code[pc];
    if(c.vtype == VALUE::Number) {
        int &_i = c.num;
        s.push(value_t(_i));
    }
    else if(c.vtype == VALUE::Char) {
        char &_c = c.ch;
        s.push(value_t(_c));
    }
    else if(c.vtype == VALUE::String) {
        std::string &_s = c.str;
        s.push(value_t(_s));
    }
    ++pc;
    goto *codetable[(int)code[pc].type];
code_pop:
    s.pop();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_add:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num + r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_sub:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num - r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_mul:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num * r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_div:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num / r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_mod:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num % r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_logor:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num || r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_logand:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num && r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_eq:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num == r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_noteq:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num != r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_lt:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num < r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_lte:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num <= r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_gt:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num > r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_gte:
    r = s.top(); s.pop();
    l = s.top(); s.pop();
    s.push(value_t(l.num >= r.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_inc:
    u = s.top(); s.pop();
    s.push(value_t(++u.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_dec:
    u = s.top(); s.pop();
    s.push(value_t(--u.num));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_store:
    c = code[pc];

    switch(c.var->var->ctype->get().type) {
        case CTYPE::INT:
            valstr = value_t(s.top().num); break;
        case CTYPE::CHAR:
            valstr = value_t(s.top().ch); break;
        case CTYPE::STRING:
            valstr = value_t(s.top().str); break;
        case CTYPE::LIST:
            valstr = value_t(s.top().listob); break;
        default:
            runtime_err("unimplemented");
    }
    s.pop();

    if(c.var->var->isglobal) {
        gvmap[c.var->var->vid] = valstr;
    }
    else {
        env.cur->vmap[c.var->var->vid] = valstr;
    }
    ++pc;
    goto *codetable[(int)code[pc].type];
code_load:
    c = code[pc];
    if(c.var->var->isglobal)
        s.push(gvmap.at(c.var->var->vid));
    else
        s.push(env.cur->vmap.at(c.var->var->vid));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_print:
    if(s.empty()) runtime_err("stack is empty at %#x", pc);

    print(s.top());
    s.pop();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_println:
    if(s.empty()) runtime_err("stack is empty at %#x", pc);

    print(s.top()); puts("");
    s.pop();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_format:
    c = code[pc];
    if(c.nfarg == 0) {
        s.push(value_t(c.str));
        ++pc;
        goto *codetable[(int)code[pc].type];
    }
    bs = c.str;
    fpos = bs.find("{}");

    while(fpos != -1) {
        ftop = [&]() -> std::string {
            switch(s.top().type) {
                case VALUE::Number:
                    return std::to_string(s.top().num);
                case VALUE::Char:
                    return std::to_string(s.top().ch);
                case VALUE::String:
                    return s.top().str;
                default:
                    error("unimplemented"); return "";
            }
        }();
        bs.replace(fpos, 2, ftop);
        fpos = bs.find("{}", fpos + ftop.length());
        s.pop();
    }

    s.push(value_t(bs));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_typeof:
    switch(s.top().ctype) {
        case CTYPE::INT:
            tyname = "int"; break;
        case CTYPE::CHAR:
            tyname = "char"; break;
        case CTYPE::STRING:
            tyname = "string"; break;
        default:
            runtime_err("unimplemented");
    }
    s.pop();
    s.push(value_t(tyname));
    ++pc;
    goto *codetable[(int)code[pc].type];
code_jmp:
    c = code[pc];
    pc = labelmap[c.str];
    ++pc;
    goto *codetable[(int)code[pc].type];
code_jmp_eq:
    c = code[pc];
    if(s.top().num == true)
        pc = labelmap[c.str];
    s.pop();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_jmp_noteq:
    c = code[pc];
    if(s.top().num == false)
        pc = labelmap[c.str];
    s.pop();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_listset:
    c = code[pc];
    for(lfcnt = 0; lfcnt < c.listsize; ++lfcnt) {
        le.push_back(s.top()); s.pop();
    }
    lsob.set_item(le);
    s.push(value_t(lsob));
    le.clear();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_fnbegin:
    c = code[pc];
    while(1) {
        ++pc;
        if(code[pc].type == OPCODE::FNEND && code[pc].str == c.str) break;
    }
    ++pc;
    goto *codetable[(int)code[pc].type];
code_call:
    c = code[pc];
    env.make();
    locs.push(pc);
    pc = labelmap[c.str];
    ++pc;
    goto *codetable[(int)code[pc].type];
code_callmethod:
    c = code[pc];
    switch(c.obmethod) {
        case Method::ListSize:
        {
            cmlsob = s.top().listob; s.pop();
            s.push(value_t((int)cmlsob.get_size()));
        } break;
        case Method::ListAccess:
        {
            cmlsob = s.top().listob; s.pop();
            int &index = s.top().num; s.pop();
            s.push(value_t(cmlsob.get_item(index)));
        } break;
        default:
            error("unimplemented");
    }
    ++pc;
    goto *codetable[(int)code[pc].type];
code_ret:
    //lvmap.clear();
    pc = locs.top(); locs.pop();
    env.escape();
    ++pc;
    goto *codetable[(int)code[pc].type];
code_label:
code_fnend:
    ++pc;
    goto *codetable[(int)code[pc].type];
code_end:
    return;
}

void VM::print(value_t &val) {
    if(val.ctype == CTYPE::INT) {
        std::cout << val.num;
    }
    else if(val.ctype == CTYPE::CHAR) {
        std::cout << val.ch;
    }
    else if(val.ctype == CTYPE::STRING) {
        std::cout << val.str;
    }
    else if(val.ctype == CTYPE::LIST) {
        printf("[ ");
        for(auto &a: val.listob.lselem) {
            if(a.ctype == CTYPE::INT)
                printf("%d", a.num);
            else if(a.ctype == CTYPE::CHAR)
                printf("'%c'", a.ch);
            else if(a.ctype == CTYPE::STRING)
                printf("\"%s\"", a.str.c_str());
            else if(a.ctype == CTYPE::LIST)
                print(a);
            printf(" ");
        }
        printf("]");
    }
}

vmenv_t *VMEnv::make() {
    vmenv_t *e = new vmenv_t();
    e->parent = cur;

    cur = e;
    return cur;
}

vmenv_t *VMEnv::escape() {
    vmenv_t *pe = cur->parent;
    cur->vmap.clear();
    delete cur;
    cur = pe;
    return cur;
}
