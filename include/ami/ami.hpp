#pragma once
#include "ast.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace ami {
double eval(const std::string& expression) {
    ami::Lexer lexer(expression);
    ami::Parser parser(lexer.lex());
    ami::Interpreter inter;
    return inter.visit(parser.parse()).val;
}
namespace literals {
double operator"" _eval(const char* expression, std::size_t) {
    return eval(expression);
}
}  // namespace literals
}  // namespace ami
