#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

enum class Tokens { Digit, Lparen, Rparen, Plus, Minus, Mult, Div };
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
    char m_Get() { return m_Src[m_Pos]; }
    void m_Advance(std::size_t x = 1) { m_Pos += x; }
    void m_AddTok(Tokens tok, const std::string& val) {
        m_Tokens.push_back(TokenHandler{.value = val, .token = tok});
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
    Lexer(const std::string& text) : m_Src(text) {}
    std::vector<TokenHandler> lex() {
        while (m_Pos < m_Src.size()) {
            switch (m_Src[m_Pos]) {
                default:
                    if (m_IsDigit(m_Get())) {
                        m_AddTok(Tokens::Digit, m_GetDigit());
                    }
                    break;
                case '*':
                    m_AddTok(Tokens::Mult, "*");
                    break;
                case '+':
                    m_AddTok(Tokens::Plus, "+");
                    break;
                case '-':
                    m_AddTok(Tokens::Minus, "-");
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
    {Tokens::Digit, "DIGIT"}};

// ASTs
enum class Op { Minus, Plus, Div, Mult };
enum class AstType { Number, BinaryOp, Expr };
std::map<Op, char> ops_str{
    {Op::Minus, '-'}, {Op::Plus, '+'}, {Op::Div, '/'}, {Op::Mult, '*'}};
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
        u_ptr out = m_ParseFactor();
        while (not_eof() && (m_Get().token == Tokens::Mult ||
                             m_Get().token == Tokens::Div)) {
            if (m_Get().token == Tokens::Mult) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(
                    Op::Mult, std::move(out), std::move(m_ParseFactor()));
            } else if (m_Get().token == Tokens::Div) {
                m_Advance();
                out = std::make_unique<BinaryOpExpr>(
                    Op::Div, std::move(out), std::move(m_ParseFactor()));
            }
        }
        return std::move(out);
    }
    u_ptr m_ParseFactor() {
        TokenHandler tok = m_Get();
        if (tok.token == Tokens::Lparen) {
            m_Advance();
            auto out = m_ParseExpr();
            if (m_Get().token != Tokens::Rparen) {
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
            m_Advance();
            return std::make_unique<Number>(std::stod(tok.value));
        }
        m_Err();
    }
    void m_Err() {
        char x[19];
        std::sprintf(x, "Syntax Error at %zu", m_Pos);
        throw std::runtime_error(x);
    }

   public:
    Parser(const std::vector<TokenHandler>& _tok) : m_Src(_tok) {}
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
                }
            }
            case AstType::Number: {
                return Number(static_cast<Number*>(expr.get())->val);
            }
        }
    }
};
