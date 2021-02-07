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
};
struct TokenHandler {
    std::string value;
    Tokens token;
    std::size_t pos;
};
// lexer
class Lexer {
    std::vector<TokenHandler> m_Tokens;
    std::size_t m_Pos = 0;
    std::string m_Src;
    bool m_IsDigit(char c) { return (c >= '0' && c <= '9'); }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_IsAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }
    char m_Get() {
        return m_Src.at((m_Pos >= m_Src.size()) ? (m_Src.size() - 1) : m_Pos);
    }
    char m_Peek(std::size_t x = 1) {
        return m_Src.at((m_Pos + x) >= m_Src.size() ? (m_Src.size() - 1)
                                                    : (m_Pos + x));
    }
    char m_Prev(std::size_t x = 1) {
        return m_Src.at((m_Pos) == 0 ? 0 : m_Pos - x);
    }
    void m_Advance(std::size_t x = 1) { m_Pos += x; }
    void m_AddTok(Tokens tok, const std::string& val) {
        m_Tokens.push_back(
            TokenHandler{.value = val, .token = tok, .pos = m_Pos});
    }
    std::string m_GetIdent() {
        std::string out{};
        while (not_eof() && m_IsAlpha(m_Get())) {
            out += m_Get();
            m_Advance();
        }
        m_Advance(-1);
        return out;
    }
    std::string m_GetDigit() {
        std::string temp{};
        while (not_eof() && m_IsDigit(m_Get())) {
            temp += m_Get();
            m_Advance();
        }
        m_Advance(-1);
        return temp;
    }
    std::string m_GetExpr() {
        std::string out{};
        while (not_eof() && m_Get() != ')') {
            m_Advance();
        }
        m_Advance(-1);
        return out;
    }

   public:
    explicit Lexer(const std::string& text) : m_Src(text) {}
    std::vector<TokenHandler> lex() {
        while (m_Pos < m_Src.size()) {
            switch (m_Src.at(m_Pos)) {
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
                    if (m_Prev() != 'e' &&
                        (m_IsDigit(m_Prev()) || m_IsDigit(m_Get()) ||
                         m_IsAlpha(m_Prev()) || m_IsAlpha(m_Get()))) {
                        // this basically says only consider it as a Minus if
                        // the prev and the current are a: ident number or ident
                        // ident or number ident or number number
                        m_AddTok(Tokens::Minus, "-");
                    } else if (m_IsAlpha(m_Peek()) && !m_IsAlpha(m_Get()) &&
                               m_Prev() != ')') {
                        // same as below but for identifiers
                        m_Advance();  // eat the '-' char
                        auto out = '-' + m_GetIdent();
                        m_AddTok(Tokens::Identifier, out);
                    } else if (m_IsDigit(m_Peek()) && !m_IsDigit(m_Get()) &&
                               m_Prev() != ')') {
                        // this basically says if the previous shit isn't an
                        // expression then consider the current number as
                        // negative number
                        m_Advance();
                        auto out = '-' + m_GetDigit();
                        m_AddTok(Tokens::Digit, out);
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
