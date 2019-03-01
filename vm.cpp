#include "maxc.h"

int VM::run(std::vector<vmcode_t> code) {
    if(code.empty())
        return 1;
    for(auto c: code)
        exec(c);
    return 0;
}

void VM::exec(vmcode_t c) {
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
        case OPCODE::PRINT: {
            if(s.empty()) runtime_err("stack is empty");

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
            if(s.empty()) runtime_err("stack is empty");

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
        default:
            error("??? exection");
    }
}
