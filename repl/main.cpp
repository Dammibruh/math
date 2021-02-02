#include <ami/ami.hpp>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

int main() {
    ami::scope_t scope;
    std::string expr;
    while ((std::cout << "> ") && std::getline(std::cin, expr)) {
        try {
            ami::EvalInterface ei{.scope = &scope,
                                  .expression = std::move(expr)};
            auto output = ami::eval(ei);
            std::cout << output << '\n';
        } catch (const std::exception& x) {
            std::cout << "err: " << x.what() << '\n';
        }
    }
}
