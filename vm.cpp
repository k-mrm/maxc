#include "maxc.h"

int VM::run(std::vector<vmcode_t> code, std::map<std::string, int> lmap) {
    if(!lmap.empty())
        labelmap = lmap;
    if(code.empty())
        return 1;
    env.cur = new vmenv_t();
    exec(code);
    /*for(auto c: code)
        exec(c); */
    return 0;
}

void VM::exec(std::vector<vmcode_t> code) {
    vmcode_t c = vmcode_t();
    value_t r, l, u;
    int _i; char _c; std::string _s;
    for(pc = 0; pc != code.size(); ++pc) {
        c = code[pc];
        switch(c.type) {
            case OPCODE::PUSH: {
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
            } break;
            case OPCODE::ADD: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num + r.num));
            } break;
            case OPCODE::SUB: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num - r.num));
            } break;
            case OPCODE::MUL: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num * r.num));
            } break;
            case OPCODE::DIV: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num / r.num));
            } break;
            case OPCODE::MOD: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num % r.num));
            } break;
            case OPCODE::LOGOR: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num || r.num));
            } break;
            case OPCODE::LOGAND: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num && r.num));
            } break;
            case OPCODE::EQ: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num == r.num));
            } break;
            case OPCODE::NOTEQ: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num != r.num));
            } break;
            case OPCODE::LT: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num < r.num));
            } break;
            case OPCODE::LTE: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num <= r.num));
            } break;
            case OPCODE::GT: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num > r.num));
            } break;
            case OPCODE::GTE: {
                r = s.top(); s.pop();
                l = s.top(); s.pop();
                s.push(value_t(l.num >= r.num));
            } break;
            case OPCODE::INC: {
                u = s.top(); s.pop();
                s.push(value_t(++u.num));
            } break;
            case OPCODE::DEC: {
                u = s.top(); s.pop();
                s.push(value_t(--u.num));
            } break;
            case OPCODE::STORE: {
                //vmap.insert(std::make_pair(c.var->var->id, s.top()));
                if(c.var->var->isglobal) {
                    gvmap[c.var->var->vid] = s.top(); s.pop();
                }
                else {
                    env.cur->vmap[c.var->var->vid] = s.top(); s.pop();
                }
            } break;
            case OPCODE::LOAD: {
                if(c.var->var->isglobal)
                    s.push(gvmap.at(c.var->var->vid));
                else
                    s.push(env.cur->vmap.at(c.var->var->vid));
            } break;
            case OPCODE::PRINT: {
                if(s.empty()) runtime_err("stack is empty at %d", pc);

                if(s.top().type == VALUE::INT) {
                    std::cout << s.top().num; s.pop();
                }
                else if(s.top().type == VALUE::CHAR) {
                    std::cout << s.top().ch; s.pop();
                }
                else if(s.top().type == VALUE::STRING) {
                    std::cout << s.top().str; s.pop();
                }
            } break;
            case OPCODE::PRINTLN: {
                if(s.empty()) runtime_err("stack is empty at %d", pc);

                if(s.top().type == VALUE::INT) {
                    std::cout << s.top().num << std::endl; s.pop();
                }
                else if(s.top().type == VALUE::CHAR) {
                    std::cout << s.top().ch << std::endl; s.pop();
                }
                else if(s.top().type == VALUE::STRING) {
                    std::cout << s.top().str << std::endl; s.pop();
                }
            } break;
            case OPCODE::TYPEOF: {
                if(s.top().type == VALUE::INT) {
                    s.pop(); s.push(value_t("int"));
                }
                else if(s.top().type == VALUE::CHAR) {
                    s.pop(); s.push(value_t("char"));
                }
                else if(s.top().type == VALUE::STRING) {
                    s.pop(); s.push(value_t("string"));
                }
            } break;
            case OPCODE::JMP: {
                pc = labelmap[c.str];
            } break;
            case OPCODE::JMP_EQ: {
                if(s.top().num == true)
                    pc = labelmap[c.str];
                s.pop();
            } break;
            case OPCODE::JMP_NOTEQ: {
                if(s.top().num == false)
                    pc = labelmap[c.str];
                s.pop();
            } break;
            case OPCODE::FNBEGIN: {
                while(1) {
                    pc++;
                    if(code[pc].type == OPCODE::FNEND && code[pc].str == c.str) break;
                }
            } break;
            case OPCODE::CALL: {
                env.make();
                locs.push(pc);
                pc = labelmap[c.str];
            } break;
            case OPCODE::RET: {
                //lvmap.clear();
                pc = locs.top(); locs.pop();
                env.escape();
            } break;
            case OPCODE::LABEL:
            case OPCODE::FNEND:
            default:
                break;
        }
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
