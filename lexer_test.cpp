#include "lexer.hpp"

#include <cstdio>

int main() {
    std::string inp{"++e"};
    Lexer lexer(inp);
    auto lexed = lexer.lex();
    for (auto& tok : lexed)
        std::printf("<token={%s}, value={%s}>\n", tokens_str[tok.token].data(),
                    tok.value.data());
}
