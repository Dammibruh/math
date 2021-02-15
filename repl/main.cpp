#include <ami/ami.hpp>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

int main() {
    std::string expr;
    while ((std::cout << "> ") && std::getline(std::cin, expr)) {
        try {
            auto output = ami::eval(expr);
            if (auto _get = std::get_if<ami::Number>(&output))
                std::cout << _get->to_str() << '\n';
            else if (auto _get = std::get_if<std::string>(&output))
                std::cout << *_get << '\n';
            else if (auto _get = std::get_if<ami::Boolean>(&output))
                std::cout << _get->raw_value << '\n';
            else if (auto _get = std::get_if<ami::NullExpr>(&output))
                std::cout << _get->value << '\n';
            else if (auto _get = std::get_if<ami::IntervalExpr>(&output))
                std::cout << _get->to_str() << '\n';
            else if (auto _get = std::get_if<ami::UnionExpr>(&output))
                std::cout << _get->to_str() << '\n';
            else if (auto _get = std::get_if<ami::InterSectionExpr>(&output))
                std::cout << _get->to_str() << '\n';
            else if (auto _get = std::get_if<ami::SetObject>(&output))
                std::cout << _get->to_str() << '\n';
        } catch (const ami::exceptions::BaseException& x) {
            std::cout << "err: " << x.what() << '\n';
        }
    }
}
