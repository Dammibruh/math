#include "lexer.hpp"

#include <cstdio>

int main() {
    std::string inp{"sqrt(6-520)*29"};
    Lexer lexer(inp);
    auto lexed = lexer.lex();
    for (auto& tok : lexed)
        std::printf("<token={%s}, value={%s}>\n", tokens_str[tok.token].data(),
                    tok.value.data());
}
