#include "main.hpp"

#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

int main() {
prog : {
    std::cout << "> ";
    std::string expr;
    if (std::getline(std::cin, expr)) {
        Lexer _lexer{expr};
        auto lexed = _lexer.lex();
        Parser _parser{lexed};
        try {
            for (auto& elm : lexed) {
                std::printf("<tok={%s}, value={%s}>\n",
                            tokens_str[elm.token].data(), elm.value.data());
            }
            auto parsed = _parser.parse();
            std::printf("%s\n", parsed->str().data());
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
