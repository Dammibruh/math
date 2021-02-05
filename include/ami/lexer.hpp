#pragma once
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace ami {
enum class Tokens {
    Digit,
    Lparen,
    Rparen,
    Plus,
    Minus,
    Dot,
    Mult,
    Div,
    Pow,
    Mod,
    Identifier,
    Delim,
    Comma,
    Unkown,
    Assign,
    Edelim,  // 1e10
    Semicolon,
    Complex
};
struct TokenHandler {
    std::string value;
    Tokens token;
};
// lexer
class Lexer {
    std::vector<TokenHandler> m_Tokens;
    std::size_t m_Pos = 0;
    std::string m_Src;
    bool m_IsDigit(char c) { return (c >= '0' && c <= '9'); }
    bool m_IsAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    char m_Get() { return m_Src[m_Pos]; }
    char m_Peek(std::size_t x = 1) { return m_Src[m_Pos + x]; }
    void m_Advance(std::size_t x = 1) { m_Pos += x; }
    char m_Prev() {
        if (m_Pos == 0)
            return m_Get();
        else
            return m_Src[m_Pos - 1];
    }
    void m_AddTok(Tokens tok, const std::string& val) {
        m_Tokens.push_back(TokenHandler{.value = val, .token = tok});
    }
    std::string m_GetIdent() {
        std::string out{};
        while (m_IsAlpha(m_Get())) {
            out += m_Get();
            m_Advance();
        }
        m_Advance(-1);
        return out;
    }
    std::string m_GetDigit() {
        std::string temp{};
        while (m_IsDigit(m_Get())) {
            temp += m_Get();
            m_Advance();
        }
        m_Advance(-1);
        return temp;
    }

   public:
    explicit Lexer(const std::string& text) : m_Src(text) {}
    std::vector<TokenHandler> lex() {
        while (m_Pos < m_Src.size()) {
            switch (m_Src[m_Pos]) {
                default:
                    if (m_IsDigit(m_Get())) {
                        // if the current pos is a digit get the full digit
                        m_AddTok(Tokens::Digit, m_GetDigit());
                    } else if (m_IsAlpha(m_Get())) {
                        m_AddTok(Tokens::Identifier, m_GetIdent());
                    } else if (!std::isspace(m_Get())) {
                        m_AddTok(Tokens::Unkown, std::string{m_Get()});
                    }
                    break;
                case '*':
                    m_AddTok(Tokens::Mult, "*");
                    break;
                case '+':
                    m_AddTok(Tokens::Plus, "+");
                    break;
                case '-':
                    // check if the previous and the next token are a
                    // ident/digit add minus token otherwise add it as a
                    // negative value
                    if (m_Src[m_Pos - 1] != 'e' &&
                        (m_IsDigit(m_Src[m_Pos - 1]) || m_IsDigit(m_Get()) ||
                         m_IsAlpha(m_Src[m_Pos - 1]) || m_IsAlpha(m_Get()))) {
                        m_AddTok(Tokens::Minus, "-");
                    } else if (m_IsAlpha(m_Peek())) {
                        m_Advance();  // eat the '-' char
                        auto out = '-' + m_GetIdent();
                        m_AddTok(Tokens::Identifier, out);
                        break;
                    } else if (m_IsDigit(m_Peek())) {
                        m_Advance();
                        auto out = '-' + m_GetDigit();
                        m_AddTok(Tokens::Digit, out);
                        break;
                    } else if (m_Peek() == '-' && (!m_IsDigit(m_Peek(2)) ||
                                                   !m_IsAlpha(m_Peek(2)))) {
                        m_AddTok(Tokens::Plus, "+");
                    } else {
                        m_AddTok(Tokens::Minus, "-");
                    }
                    break;
                case '/':
                    m_AddTok(Tokens::Div, "/");
                    break;
                case '(':
                    m_AddTok(Tokens::Lparen, "(");
                    break;
                case ')':
                    m_AddTok(Tokens::Rparen, ")");
                    break;
                case '^':
                    m_AddTok(Tokens::Pow, "^");
                    break;
                case '%':
                    m_AddTok(Tokens::Mod, "%");
                    break;
                case ',':
                    // for function args
                    m_AddTok(Tokens::Comma, ",");
                    break;
                case '.':
                    // for decimals
                    m_AddTok(Tokens::Dot, ".");
                    break;
                case '\'':
                    // delim for number to improve readability eg :
                    // 1'000'000'000
                    m_AddTok(Tokens::Delim, "'");
                    break;
                case 'e':
                    if (m_IsAlpha(m_Peek())) {
                        m_AddTok(Tokens::Identifier, m_GetIdent());
                    } else {
                        m_AddTok(Tokens::Edelim, "e");
                    }
                    break;
                case '=':
                    m_AddTok(Tokens::Assign, "=");
                    break;
                case ';':
                    m_AddTok(Tokens::Semicolon, ";");
                    break;
            }
            m_Advance();
        }
        return m_Tokens;
    }
};
static std::map<Tokens, std::string_view> tokens_str{
    {Tokens::Div, "DIV"},
    {Tokens::Mult, "MULT"},
    {Tokens::Plus, "PLUS"},
    {Tokens::Minus, "MINUS"},
    {Tokens::Lparen, "LPAREN"},
    {Tokens::Rparen, "RPAREN"},
    {Tokens::Digit, "DIGIT"},
    {Tokens::Pow, "POW"},
    {Tokens::Mod, "MOD"},
    {Tokens::Comma, "COMMA"},
    {Tokens::Identifier, "IDENTIFIER"},
    {Tokens::Delim, "DELIM"},
    {Tokens::Dot, "DOT"},
    {Tokens::Unkown, "UNKOWN"},
    {Tokens::Edelim, "EDELIM"},
    {Tokens::Assign, "ASSIGN"},
    {Tokens::Semicolon, "SEMICOLON"}};

}  // namespace ami
