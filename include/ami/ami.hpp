#pragma once
#include <string>

#include "ast.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace ami {
std::variant<Number, Boolean, NullExpr, IntervalExpr, std::string> eval(
    const std::string& expression, const std::string& file = "source") {
    ami::Lexer lexer(expression);
    ami::Parser parser(lexer.lex(), expression, file);
    auto parsed = parser.parse();
    ami::Interpreter inter(parser.get_ei());
    return inter.visit(parsed);
}
}  // namespace ami
