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
            if (auto _out = std::get_if<ami::Boolean>(&output))
                std::cout << _out->val << '\n';
            else if (auto _out = std::get_if<ami::Number>(&output))
                std::cout << _out->val << '\n';
            else if (auto _out = std::get_if<std::string>(&output))
                std::cout << *_out << '\n';
        } catch (const ami::exceptions::BaseException& x) {
            std::cout << "err: " << x.what() << '\n';
        }
    }
}
