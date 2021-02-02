#include <ami/ami.hpp>
#include <iostream>

int main() {
    // basic math expressions
    std::string expression = "5^-1*10";
    std::string expression2 = "4*pi*(5^2)";
    ami::EvalInterface ei{.expression = expression};
    ami::EvalInterface ei2{.expression = expression2};
    auto out = ami::eval(ei);
    auto out2 = ami::eval(ei2);
    std::cout << "out: " << out << '\n' << "out2: " << out2 << '\n';
}
