#include "main.hpp"

int main() {
prog : {
    std::cout << "> ";
    std::string expr;
    if (std::getline(std::cin, expr)) {
        Lexer _lexer{expr};
        auto lexed = _lexer.lex();
        Parser _parser{lexed};
        try {
            auto parsed = _parser.parse();
            Interpreter inter;
            auto output = inter.visit(std::move(parsed));
            std::cout << output.val << '\n';
            goto prog;
        } catch (const std::exception& x) {
            std::cout << "err " << x.what() << '\n';
            goto prog;
        }
    }
}
}
