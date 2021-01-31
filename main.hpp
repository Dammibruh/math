#include <cmath>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class Tokens {
    Digit,
    Lparen,
    Rparen,
    Plus,
    Minus,
    Mult,
    Div,
    Pow,
    Mod,
    Sqrt
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
    bool m_IsDigit(char c) { return (c >= '0' && c <= '9') || c == '.'; }
    char m_Get() { return m_Src[m_Pos]; }
    char m_Peek() { return m_Src[m_Pos + 1]; }
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
    std::string m_GetDigit() {
        std::string temp{};
        while (m_IsDigit(m_Get())) {
            if (m_Get() == '.' && temp.find('.') == std::string::npos) {
                temp += '.';
                m_Advance();
            }
            temp += m_Get();
            m_Advance();
        }
        m_Advance(-1);
        return temp;
    }

    bool m_IsSqrt(char a, char b) {
        char t[2];
        t[0] = a;
        t[1] = b;
        return std::strcmp(t, "\u221A") == 0;
    }

   public:
    Lexer(const std::string& text) : m_Src(text) {}
    std::vector<TokenHandler> lex() {
        while (m_Pos < m_Src.size()) {
            switch (m_Src[m_Pos]) {
                default:
                    if (m_IsDigit(m_Get())) {
                        m_AddTok(Tokens::Digit, m_GetDigit());
                    }
                    break;
                case 'x':
                    m_AddTok(Tokens::Mult, "x");
                    break;
                case '+':
                    m_AddTok(Tokens::Plus, "+");
                    break;
                case '-':
                    if (m_IsDigit(m_Src[m_Pos - 1]) || m_IsDigit(m_Get())) {
                        m_AddTok(Tokens::Minus, "-");
                    } else if (!m_IsDigit(m_Prev())) {
                        m_Advance();
                        auto out = '-' + m_GetDigit();
                        m_AddTok(Tokens::Digit, out);
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
            }
            m_Advance();
        }
        return m_Tokens;
    }
};
std::map<Tokens, std::string_view> tokens_str{
    {Tokens::Div, "DIV"},       {Tokens::Mult, "MULT"},
    {Tokens::Plus, "PLUS"},     {Tokens::Minus, "MINUS"},
    {Tokens::Lparen, "LPAREN"}, {Tokens::Rparen, "RPAREN"},
    {Tokens::Digit, "DIGIT"},   {Tokens::Pow, "POW"},
    {Tokens::Mod, "MOD"},       {Tokens::Sqrt, "SQRT"}};

// ASTs
enum class Op { Minus, Plus, Div, Mult, Pow, Mod, Sqrt };
enum class AstType { Number, BinaryOp, Expr };
std::map<Op, char> ops_str{
    {Op::Minus, '-'}, {Op::Plus, '+'}, {Op::Div, '/'},       {Op::Mult, 'x'},
    {Op::Pow, '^'},   {Op::Mod, '%'},  {Op::Sqrt, u'\u221A'}};
struct Expr {
    virtual std::string str() = 0;
    virtual AstType type() const { return AstType::Expr; }
    virtual ~Expr() = default;
};
struct Number : Expr {
    double val;
    Number(double x) : val(x) {}
    std::string str() override {
        char ch[19];
        std::sprintf(ch, "<NUMBER:%d>", val);
        std::string _val{ch};
        return _val;
    }
    AstType type() const override { return AstType::Number; }
};
struct BinaryOpExpr : public Expr {
    std::unique_ptr<Expr> lhs, rhs;
    Op op;
    BinaryOpExpr(Op c, std::unique_ptr<Expr> _lhs, std::unique_ptr<Expr> _rhs)
        : lhs(std::move(_lhs)), rhs(std::move(_rhs)), op(c) {}
    std::string str() override {
        std::stringstream ss;
        std::string lhs_str = lhs != nullptr ? lhs->str() : "null";
        std::string rhs_str = rhs != nullptr ? rhs->str() : "null";
        ss << "<BinaryOpExpr Lhs={" << lhs_str << "}, OP={" << ops_str[op]
           << "}, Rhs={" << rhs_str << "}/>";
        return ss.str();
    }
    AstType type() const override { return AstType::BinaryOp; }
};
// parser
class Parser {
    using u_ptr = std::unique_ptr<Expr>;
    std::vector<TokenHandler> m_Src;
    std::size_t m_Pos = 0;

    TokenHandler m_Get() { return m_Src[m_Pos]; }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_IsDigit(const std::string& str) {
        std::string::size_type count = 0;
        for (auto& c : str)
            if (std::isdigit(c) || c == '-' || c == '+') count++;
        return count == str.size();
    }
    void m_Advance(std::size_t x = 1) { m_Pos += x; }
    u_ptr m_ParseExpr() {
        u_ptr out = m_ParseTerm();
        while (not_eof() && (m_Get().token == Tokens::Plus ||
                             m_Get().token == Tokens::Minus)) {
            if (m_Get().token == Tokens::Plus) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(Op::Plus, std::move(out),
                                                     std::move(m_ParseTerm()));
            } else if (m_Get().token == Tokens::Minus) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(Op::Minus, std::move(out),
                                                     std::move(m_ParseTerm()));
            }
        }
        return std::move(out);
    }
    u_ptr m_ParseTerm() {
        u_ptr out = m_ParseSu();
        while (not_eof() && (m_Get().token == Tokens::Mult ||
                             m_Get().token == Tokens::Div)) {
            if (m_Get().token == Tokens::Mult) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(Op::Mult, std::move(out),
                                                     std::move(m_ParseSu()));
            } else if (m_Get().token == Tokens::Div) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(Op::Div, std::move(out),
                                                     std::move(m_ParseSu()));
            }
        }
        return std::move(out);
    }
    u_ptr m_ParseSu() {
        u_ptr out = m_ParseFactor();
        while (not_eof() &&
               (m_Get().token == Tokens::Pow || m_Get().token == Tokens::Mod)) {
            if (m_Get().token == Tokens::Pow) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(
                    Op::Pow, std::move(out), std::move(m_ParseFactor()));
            } else if (m_Get().token == Tokens::Mod) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(
                    Op::Mod, std::move(out), std::move(m_ParseFactor()));
            }
        }
        return std::move(out);
    }
    u_ptr m_ParseFactor() {
        TokenHandler tok = m_Get();
        if (tok.token == Tokens::Lparen) {
            m_Advance();
            auto out = m_ParseExpr();
            if (m_Get().token != Tokens::Rparen || !not_eof()) {
                m_Err();
            }
            m_Advance();
            return out;
        } else if (tok.token == Tokens::Plus) {
            m_Advance();
            return std::make_unique<BinaryOpExpr>(
                Op::Plus, std::move(m_ParseFactor()), nullptr);
        } else if (tok.token == Tokens::Minus) {
            m_Advance();
            return std::make_unique<BinaryOpExpr>(
                Op::Minus, std::move(m_ParseFactor()), nullptr);
        } else if (tok.token == Tokens::Digit) {
            if (m_IsDigit(tok.value) || tok.value.size() > 0 || not_eof()) {
                m_Advance();
                return std::make_unique<Number>(std::stod(tok.value));
            } else {
                m_Err();
            }
        }
        m_Err();
    }
    void m_Err() {
        std::stringstream ss;
        ss << "Syntax Error at " << m_Pos << "invalid expression \""
           << m_Get().value << '"';
        throw std::runtime_error(ss.str());
    }

   public:
    Parser(const std::vector<TokenHandler>& _tok) {
        if (_tok.size() > 0)
            m_Src = _tok;
        else
            throw std::runtime_error("invalid input");
    }
    u_ptr parse() { return std::move(m_ParseExpr()); }
};

class Interpreter {
    using u_ptr = std::unique_ptr<Expr>;
    Number m_VisitAdd(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val +
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitSub(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val -
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitDiv(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val /
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitMult(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val *
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitMod(BinaryOpExpr* boe) {
        return Number(std::fmod(visit(std::move(boe->lhs)).val,
                                visit(std::move(boe->rhs)).val));
    }
    Number m_VisitPow(BinaryOpExpr* boe) {
        return Number(std::pow(visit(std::move(boe->lhs)).val,
                               visit(std::move(boe->rhs)).val));
    }
    Number m_VisitSqrt(BinaryOpExpr* boe) {
        return Number(std::sqrt(visit(std::move(boe->lhs)).val));
    }
    Number m_VisitNumber(Number* num) { return Number(num->val); }

   public:
    Interpreter() = default;
    Number visit(u_ptr expr) {
        switch (expr->type()) {
            default: {
                throw std::runtime_error("invalid expr");
                break;
            }
            case AstType::BinaryOp: {
                BinaryOpExpr* bopexpr = static_cast<BinaryOpExpr*>(expr.get());
                switch (bopexpr->op) {
                    default:
                        throw std::runtime_error("invalid operator");
                        break;
                    case Op::Plus:
                        return m_VisitAdd(bopexpr);
                    case Op::Minus:
                        return m_VisitSub(bopexpr);
                    case Op::Div:
                        return m_VisitDiv(bopexpr);
                    case Op::Mult:
                        return m_VisitMult(bopexpr);
                    case Op::Pow:
                        return m_VisitPow(bopexpr);
                    case Op::Mod:
                        return m_VisitMod(bopexpr);
                    case Op::Sqrt:
                        return m_VisitSqrt(bopexpr);
                }
            }
            case AstType::Number: {
                return Number(static_cast<Number*>(expr.get())->val);
            }
        }
    }
};

