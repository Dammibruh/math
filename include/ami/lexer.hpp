#pragma once
#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace ami {
enum class Tokens {
    Digit,
    Lparen,
    Rparen,
    Plus,
    PlusAssign,
    Minus,
    MinusAssign,
    Dot,
    Mult,
    MultAssign,
    Div,
    DivAssign,
    Pow,
    PowAssign,
    Mod,
    ModAssign,
    Identifier,
    Delim,
    Comma,
    Unkown,
    Assign,
    Edelim,  // 1e10
    Semicolon,
    Lcbracket,  // for intervals
    Rcbracket,
    KeywordIf,
    KeywordElse,
    KeywordReturn,
    KeywordUnion,
    KeywordIntersection,
    KeywordAnd,
    KeywordNot,
    KeywordNull,
    KeywordIn,
    KeywordOr,
    FunctionDef,  // f(x) -> expr
    Equals,
    NotEquals,
    GreaterThan,
    Lbracket,  // for sets
    Factorial,
    Rbracket,
    GreaterThanOrEqual,
    LessThan,
    Range,
    LessThanOrEqual,
    Boolean
};
struct TokenHandler {
    std::string value;
    Tokens token;
    std::size_t pos;
    template <class... Args>
    bool is(Args&&... args) {
        for (auto& e : {(args)...})
            if (e == this->token) return true;
        return false;
    }
    template <class... Args>
    bool isNot(Args&&... args) {
        for (auto& e : {(args)...})
            if (e != this->token) return true;
        return false;
    }
};
// lexer
class Lexer {
    std::vector<TokenHandler> m_Tokens;
    std::size_t m_Pos = 0;
    std::string m_Src;
    bool m_IsDigit(char c) { return (c >= '0' && c <= '9'); }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_AtEnd() { return (m_Pos == (m_Src.size() - 1)); }
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
    Tokens m_GetKeyword(const std::string& ident) {
        // if (condition) stmt1 else stmt2
        if (ident == "if") {
            return Tokens::KeywordIf;
        } else if (ident == "else") {
            return Tokens::KeywordElse;
        } else if (ident == "true" || ident == "false") {
            return Tokens::Boolean;
        } else if (ident == "and") {
            return Tokens::KeywordAnd;
        } else if (ident == "or") {
            return Tokens::KeywordOr;
        } else if (ident == "not") {
            return Tokens::KeywordNot;
        } else if (ident == "in") {
            return Tokens::KeywordIn;
        } else if (ident == "null") {
            return Tokens::KeywordNull;
        } else if (ident == "return") {
            return Tokens::KeywordReturn;
        } else if (ident == "union") {
            return Tokens::KeywordUnion;
        } else if (ident == "intersection") {
            return Tokens::KeywordIntersection;
        } else {
            return Tokens::Identifier;
        }
    }
    void m_Advance(std::size_t x = 1) { m_Pos += x; }
    void m_AddTok(Tokens tok, const std::string& val) {
        m_Tokens.push_back(
            TokenHandler{.value = val, .token = tok, .pos = m_Pos});
    }
    std::string m_GetIdent() {
        std::string out{};
        while (not_eof() && (m_IsAlpha(m_Get()) || m_IsDigit(m_Get()))) {
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

   public:
    explicit Lexer(const std::string& text) : m_Src(text) {}
    std::vector<TokenHandler> lex() {
        while (m_Pos < m_Src.size()) {
            switch (m_Src.at(m_Pos)) {
                default:
                    if (m_IsDigit(m_Get())) {
                        m_AddTok(Tokens::Digit, m_GetDigit());
                    } else if (m_IsAlpha(m_Get())) {
                        std::string ident = m_GetIdent();
                        m_AddTok(m_GetKeyword(ident), ident);
                    } else if (!std::isspace(m_Get())) {
                        m_AddTok(Tokens::Unkown, std::string{m_Get()});
                    }
                    break;
                case '*':
                    if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::MultAssign, "*=");
                    }
                    m_AddTok(Tokens::Mult, "*");
                    break;
                case '+':
                    if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::PlusAssign, "+=");
                    } else {
                        m_AddTok(Tokens::Plus, "+");
                    }
                    break;
                case '-':
                    if (m_Peek() == '>') {
                        m_Advance();
                        m_AddTok(Tokens::FunctionDef, "->");
                    } else if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::MinusAssign, "-=");

                    } else {
                        m_AddTok(Tokens::Minus, "-");
                    }
                    break;
                case '/':
                    if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::DivAssign, "/=");
                    } else {
                        m_AddTok(Tokens::Div, "/");
                    }
                    break;
                case '(':
                    m_AddTok(Tokens::Lparen, "(");
                    break;
                case ')':
                    m_AddTok(Tokens::Rparen, ")");
                    break;
                case '^':
                    if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::PowAssign, "^=");
                    } else {
                        m_AddTok(Tokens::Pow, "^");
                    }
                    break;
                case '%':
                    if (m_Peek() == '=') {
                        m_Advance();
                        m_AddTok(Tokens::ModAssign, "%=");
                    } else {
                        m_AddTok(Tokens::Mod, "%");
                    }
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
                    // delim for number to improve readability like :
                    // 1'000'000'000
                    m_AddTok(Tokens::Delim, "'");
                    break;
                case 'e':
                    if (m_IsAlpha(m_Peek()) && not_eof()) {
                        std::string ident = m_GetIdent();
                        m_AddTok(m_GetKeyword(ident), ident);
                    } else if (m_IsDigit(m_Prev())) {
                        m_AddTok(Tokens::Edelim, "e");
                    } else {
                        m_AddTok(Tokens::Identifier, "e");
                    }
                    break;
                case '=':
                    if (m_Peek() == '=' && !m_AtEnd()) {
                        m_Advance();
                        m_AddTok(Tokens::Equals, "==");
                    } else {
                        m_AddTok(Tokens::Assign, "=");
                    }
                    break;
                case '<':
                    if (m_Peek() == '=' && not_eof()) {
                        m_Advance();
                        m_AddTok(Tokens::LessThanOrEqual, "<=");
                    } else {
                        m_AddTok(Tokens::LessThan, "<");
                    }
                    break;
                case '>':
                    if (m_Peek() == '=' && not_eof()) {
                        m_Advance();
                        m_AddTok(Tokens::GreaterThanOrEqual, ">=");
                    } else {
                        m_AddTok(Tokens::GreaterThan, ">");
                    }
                    break;
                case '[':
                    m_AddTok(Tokens::Lcbracket, "[");
                    break;
                case ']':
                    m_AddTok(Tokens::Rcbracket, "]");
                    break;
                case ';':
                    m_AddTok(Tokens::Semicolon, ";");
                    break;
                case '{':
                    m_AddTok(Tokens::Lbracket, "{");
                    break;
                case '}':
                    m_AddTok(Tokens::Rbracket, "}");
                    break;
                case '!':
                    if (m_Peek() == '=' && !m_AtEnd()) {
                        m_Advance();
                        m_AddTok(Tokens::NotEquals, "!=");
                    } else {
                        m_AddTok(Tokens::Factorial, "!");
                    }
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
    {Tokens::DivAssign, "DIVASSIGN"},
    {Tokens::MultAssign, "MULTASSIGN"},
    {Tokens::PlusAssign, "PLUSASSIGN"},
    {Tokens::MinusAssign, "MINUSASSIGN"},
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
    {Tokens::Semicolon, "SEMICOLON"},
    {Tokens::Boolean, "BOOLEAN"},
    {Tokens::KeywordIf, "KEYWORDIF"},
    {Tokens::KeywordUnion, "KEYWORDUNION"},
    {Tokens::KeywordIn, "KEYWORDIN"},
    {Tokens::KeywordElse, "KEYWORDELSE"},
    {Tokens::Lbracket, "LBRACKET"},
    {Tokens::Rbracket, "RBRACKET"},
    {Tokens::GreaterThan, "GREATERTHAN"},
    {Tokens::GreaterThanOrEqual, "GREATERTHANOREQUAL"},
    {Tokens::LessThan, "LESSTHAN"},
    {Tokens::LessThanOrEqual, "LESSTHANOREQUAL"},
    {Tokens::Equals, "EQUALS"},
    {Tokens::NotEquals, "NOTEQUALS"},
    {Tokens::KeywordOr, "KEYWORDOR"},
    {Tokens::KeywordAnd, "KEYWORDAND"},
    {Tokens::KeywordNull, "KEYWORDNULL"},
    {Tokens::KeywordNot, "KEYWORDNOT"},
    {Tokens::FunctionDef, "FUNCTIONDEF"},
    {Tokens::Lcbracket,
     "LEFTCUBEBRACKET"},  // lmao idk from where did I get this name
    {Tokens::Rcbracket, "RIGHTCUBEBRACKET"},
};

}  // namespace ami
