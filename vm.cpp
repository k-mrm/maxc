#include "maxc.h"

int VM::run(std::vector<vmcode_t> code, std::map<std::string, int> lmap) {
    if(!lmap.empty())
        labelmap = lmap;
    if(code.empty())
        return 1;
    env.cur = new vmenv_t();
    exec(code);
    return 0;
}

void VM::exec(std::vector<vmcode_t> code) {
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
        &&code_ret,
        &&code_call,
        &&code_fnbegin,
        &&code_fnend,
        &&code_end,
    };
    pc = 0;
    vmcode_t c = vmcode_t();
    value_t r, l, u;    //binary, unart
    value_t valstr;     //variable store
    int _i; char _c; std::string _s;    //stack push
    std::string _format; int fpos; std::string bs; std::string ftop; //format

    goto *codetable[(int)code[pc].type];

code_push:
    c = code[pc];
    if(c.vtype == VALUE::INT) {
        _i = c.value;
        s.push(value_t(_i));
    }
    else if(c.vtype == VALUE::CHAR) {
        _c = c.ch;
        s.push(value_t(_c));
    }
    else if(c.vtype == VALUE::STRING) {
        _s = c.str;
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
    valstr = [&]() -> value_t {
        switch(c.var->var->ctype->get().type) {
            case CTYPE::INT:
                return value_t(s.top().num);
            case CTYPE::CHAR:
                return value_t(s.top().ch);
            case CTYPE::STRING:
                return value_t(s.top().str);
            default:
                runtime_err("umimplemented"); return value_t();
        }
    }();
    s.pop();

    if(c.var->var->isglobal)
        gvmap[c.var->var->vid] = valstr;
    else
        env.cur->vmap[c.var->var->vid] = valstr;
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

    if(s.top().type == VALUE::INT) {
        std::cout << s.top().num; s.pop();
    }
    else if(s.top().type == VALUE::CHAR) {
        std::cout << s.top().ch; s.pop();
    }
    else if(s.top().type == VALUE::STRING) {
        std::cout << s.top().str; s.pop();
    }
    ++pc;
    goto *codetable[(int)code[pc].type];
code_println:
    if(s.empty()) runtime_err("stack is empty at %#x", pc);

    if(s.top().type == VALUE::INT) {
        std::cout << s.top().num << std::endl; s.pop();
    }
    else if(s.top().type == VALUE::CHAR) {
        std::cout << s.top().ch << std::endl; s.pop();
    }
    else if(s.top().type == VALUE::STRING) {
        std::cout << s.top().str << std::endl; s.pop();
    }
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
                case VALUE::INT:
                    return std::to_string(s.top().num);
                case VALUE::CHAR:
                    return std::to_string(s.top().ch);
                case VALUE::STRING:
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
    if(s.top().type == VALUE::INT) {
        s.pop(); s.push(value_t("int"));
    }
    else if(s.top().type == VALUE::CHAR) {
        s.pop(); s.push(value_t("char"));
    }
    else if(s.top().type == VALUE::STRING) {
        s.pop(); s.push(value_t("string"));
    }
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
