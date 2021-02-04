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
            std::cout << output << '\n';
        } catch (const std::exception& x) {
            std::cout << "err: " << x.what() << '\n';
        }
    }
}
