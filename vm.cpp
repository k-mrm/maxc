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
    for(pc = 0; pc < code.size(); pc++) {
        auto c = code[pc];
        switch(c.type) {
            case OPCODE::PUSH: {
                if(c.vtype == VALUE::INT) {
                    int a = c.value;
                    s.push(value_t(a));
                }
                else if(c.vtype == VALUE::CHAR) {
                    char _c = c.ch;
                    s.push(value_t(_c));
                }
                else if(c.vtype == VALUE::STRING) {
                    std::string _s = c.str;
                    s.push(value_t(_s));
                }
            } break;
            case OPCODE::ADD: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num + r.num));
            } break;
            case OPCODE::SUB: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num - r.num));
            } break;
            case OPCODE::MUL: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num * r.num));
            } break;
            case OPCODE::DIV: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num / r.num));
            } break;
            case OPCODE::MOD: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num % r.num));
            } break;
            case OPCODE::LOGOR: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num || r.num));
            } break;
            case OPCODE::LOGAND: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num && r.num));
            } break;
            case OPCODE::EQ: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num == r.num));
            } break;
            case OPCODE::NOTEQ: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num != r.num));
            } break;
            case OPCODE::LT: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num < r.num));
            } break;
            case OPCODE::LTE: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num <= r.num));
            } break;
            case OPCODE::GT: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num > r.num));
            } break;
            case OPCODE::GTE: {
                auto r = s.top(); s.pop();
                auto l = s.top(); s.pop();
                s.push(value_t(l.num >= r.num));
            } break;
            case OPCODE::INC: {
                auto u = s.top(); s.pop();
                s.push(value_t(++u.num));
            } break;
            case OPCODE::DEC: {
                auto u = s.top(); s.pop();
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
    cur->vmap.clear();
    cur = cur->parent;
    return cur;
}
