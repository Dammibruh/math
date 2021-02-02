#include <ami/ami.hpp>
#include <iostream>

int main() {
    // basic math expressions
    ami::scope_t scope;
    std::string expression = "5^-1*10";
    std::string expression2 = "4*pi*(5^2)";
    std::string expression3 = "pi";
    ami::EvalInterface ei{.expression = expression};
    ami::EvalInterface ei2{.expression = expression2};
    ami::EvalInterface ei3{.scope = &scope, .expression = expression3};
    auto out = ami::eval(ei);
    auto out2 = ami::eval(ei2);
    auto out3 = ami::eval(ei3);
    std::cout << "out: " << out << '\n'
              << "out2: " << out2 << '\n'
              << "out3: " << out3 << '\n';
}
