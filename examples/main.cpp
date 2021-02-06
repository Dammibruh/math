#include <ami/ami.hpp>
#include <iostream>

int main() {
    // basic math expressions
    std::string expression("5^-1*10");
    std::string expression2("4*pi*(5^2)");
    std::string functions("sqrt(25)");
    auto out = ami::eval(expression);
    auto out2 = ami::eval(expression2);
    using namespace ami::literals;  // can also use string literals
    std::cout << expression << ": " << out << '\n'
              << expression2 << ": " << out2 << '\n'
              << "pi"
              << ": "
              << "pi"_eval << '\n'
              << functions << ": " << ami::eval(functions) << '\n';
}
