#pragma once
#include "ast.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace ami {
using scope_t = std::map<std::string, double>;
struct EvalInterface {
    scope_t* scope = nullptr;
    std::string expression;
};
double eval(const EvalInterface& ei) {
    ami::Lexer lexer(ei.expression);
    ami::Parser parser(lexer.lex());
    ami::Interpreter inter(ei.scope);
    return inter.visit(std::move(parser.parse())).val;
}
}  // namespace ami
