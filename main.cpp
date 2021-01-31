#include "main.hpp"

int main() {
    std::string expr{"(62-29)*10/48*(473+10)"};
    std::cout << "lexing...\n";
    Lexer _lexer{expr};
    auto lexed = _lexer.lex();
    decltype(lexed)::size_type index = 0;
    /* for (auto& t : lexed) {
         std::printf("<tok={%s}, val={%s}, pos={%zu}>\n",
                     tokens_str[t.token].data(), t.value.data(), index);
         index++;
     }*/
    std::cout << "parsing...\n";
    Parser _parser{lexed};
    try {
        auto parsed = _parser.parse();
        // std::cout << parsed->str() << '\n';
        std::cout << "interpreting...\n";
        Interpreter inter;
        auto output = inter.visit(std::move(parsed));
        std::cout << "input > " << expr << " result: " << output.val;
    } catch (const std::exception& x) {
        std::cout << "err " << x.what();
    }
}
