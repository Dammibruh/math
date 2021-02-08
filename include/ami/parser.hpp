#pragma once
#include <fmt/core.h>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ast.hpp"
#include "errors.hpp"
namespace ami {
/*
 * TODO:
 * use vector of Exprs for function body to parse full source file
 * wihtout ignoring previous parsed epxressions
 * */
class Parser {
    using ptr_t = std::shared_ptr<Expr>;
    std::vector<TokenHandler> m_Src;
    std::size_t m_Pos = 0;
    std::size_t m_ParensCount = 0;
    ami::exceptions::ExceptionInterface ei;

    TokenHandler m_Get() {
        return m_Src.at(m_Pos >= m_Src.size() ? m_Src.size() - 1 : m_Pos);
    }
    TokenHandler m_Prev(std::size_t x = 1) {
        return m_Src.at((m_Pos) == 0 ? 0 : m_Pos - x);
    }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_IsDigit(const std::string& str) {
        for (auto& c : str)
            if (!std::isdigit(c)) return false;
        return true;
    }
    bool m_IsAnOp(Tokens tok) {
        return (tok == Tokens::Mod) || (tok == Tokens::Div) ||
               (tok == Tokens::Mult) || (tok == Tokens::Plus) ||
               (tok == Tokens::Minus) || (tok == Tokens::Pow);
    }
    bool m_IsValidAfterNumber(TokenHandler tok) {
        return m_IsAnOp(tok.token) || (tok.token == Tokens::Lparen) ||
               (tok.token == Tokens::Rparen) || (tok.token == Tokens::Digit) ||
               (tok.token == Tokens::Delim) || (tok.token == Tokens::Edelim);
    }
    ptr_t m_ParseIfStmtBody() {}
    ptr_t m_ParseIfStmtStmt() {}
    std::vector<ptr_t> m_ParseFunctionArgs() {
        std::vector<ptr_t> args{};
        if (m_Get().token != Tokens::Rparen) {
            do {
                if (m_Get().token == Tokens::Comma) {
                    // to prevent weird syntaxes and eofs while parsing from
                    // being valid e.i: func(,) or func(, or func(5, etc
                    if (args.size() > 0 && m_Peek().token != Tokens::Rparen &&
                        not_eof())
                        m_Advance();
                    else
                        m_Err();
                } else if (!not_eof()) {
                    m_ThrowErr("ParseError",
                               "EOF while parsing function arguments ");
                } else {
                    args.push_back(m_ParseExpr());
                }
            } while (m_Get().token != Tokens::Rparen);
        }
        if (m_Get().token != Tokens::Rparen) {
            m_ThrowErr("SyntaxError", "expected ')' after arguments list");
        }
        m_Advance();
        return args;
    }
    std::vector<ptr_t> m_ParseFunctionDefArgs() {
        std::vector<ptr_t> args{};
        if (m_Get().token != Tokens::Rparen) {
            do {
                if (m_Get().token == Tokens::Comma) {
                    if (args.size() > 0 && m_Peek().token != Tokens::Rparen &&
                        not_eof())
                        m_Advance();
                    else
                        m_Err();
                } else if (!not_eof()) {
                    m_ThrowErr("ParseError",
                               "EOF while parsing function arguments ");
                } else {
                    ptr_t temp = m_ParseExpr();
                    if (temp->type() == AstType::Identifier) {
                        args.push_back(temp);
                    } else {
                        // to allow only identifier in arguments when defining a
                        // function i.e: this is invalid `f(5) = x`
                        m_ThrowErr(
                            "TypeError",
                            "expected identifier in function's arguments");
                    }
                }
            } while (m_Get().token != Tokens::Rparen);
        }
        if (m_Get().token != Tokens::Rparen) {
            m_ThrowErr("SyntaxError", "expected ')' after arguments list");
        }
        m_Advance();
        return args;
    }
    ptr_t m_ParseFunctionDefOrCall(TokenHandler tok) {
        /*
         * create a subvector from the main source so
         * helper variables won't look for out of
         * range tokens i.e:
         * m_Src have a function definition that's
         * call another function: 'func(x) =
         * sqrt(x)+x`, get_rparen and get_assign will
         * look up at m_Src range for rparen and
         * assign tokens
         * ) and = for function 'func' will be found
         * and a unexpected sigfaults will happen
         */
        std::string name = tok.value;
        std::vector<TokenHandler> src(m_Src.begin() + m_Pos, m_Src.end());
        auto get_rparen = std::find_if(src.begin(), src.end(), [](auto t) {
            return t.token == Tokens::Rparen;
        });
        auto get_assign = std::find_if(src.begin(), src.end(), [](auto t) {
            return t.token == Tokens::Assign;
        });
        bool contains_assign = get_assign != src.end();
        bool invalid_assign = get_assign < get_rparen;
        std::vector<ptr_t> args;
        if (contains_assign && !invalid_assign) {
            args = m_ParseFunctionDefArgs();
        } else if (!contains_assign) {
            args = m_ParseFunctionArgs();
        } else {
            m_Err();
        }
        m_Advance();
        if (contains_assign) {
            ptr_t body = m_ParseExpr();
            m_Advance(-1);
            // so the parser won't ignore operations after the
            // closed rparen ')' since we're advancing each
            // function argument and advancing again after
            // parsing the arguments

            return std::make_shared<Function>(name, body, args);
        } else {
            m_Advance(-1);
            return std::make_shared<FunctionCall>(name, args);
        }
    }
    ptr_t m_ParseIdentAssign() {
        ptr_t value = m_ParseExpr();
        while (not_eof() && m_Get().token != Tokens::Semicolon) {
            m_Advance();
            value = m_ParseExpr();
        }
        return value;
    }
    bool m_IsValidPunc(TokenHandler tok) {
        return (tok.token == Tokens::Digit || tok.token == Tokens::Dot ||
                tok.token == Tokens::Delim || tok.token == Tokens::Edelim ||
                tok.token == Tokens::Minus);
    }
    std::string m_GetDigit() {
        std::string temp{};
        bool is_decimal{};
        bool contains_e{};
        while (not_eof() && m_IsValidPunc(m_Get())) {
            if (m_Get().token == Tokens::Digit) {
                temp += m_Get().value;
            } else if (m_Get().token == Tokens::Dot) {
                if (is_decimal) {
                    m_Err();
                } else {
                    temp += m_Get().value;
                    is_decimal = true;
                }
            } else if (m_Get().token == Tokens::Edelim) {
                if (contains_e) {
                    m_Err();
                } else {
                    temp += m_Get().value;
                    contains_e = true;
                }
            } else if (m_Get().token == Tokens::Minus) {
                if (contains_e && m_Prev().token == Tokens::Edelim) {
                    temp += m_Get().value;
                    // if the digit is 1e-10 parse the e-10 then break cuz we
                    // don't want to ignore the minus operator
                } else {
                    break;
                }
            }
            m_Advance();
        }
        return temp;
    }
    TokenHandler m_Peek(std::size_t x = 1) {
        return m_Src.at((m_Pos + x) >= m_Src.size() ? m_Src.size() - 1
                                                    : m_Pos + x);
    }
    void m_Advance(std::size_t x = 1) {
        if (m_Pos < m_Src.size())
            m_Pos += x;
        else
            return;
    }
    bool m_IsCompareToken(Tokens tok) {
        return (tok == Tokens::GreaterThan) ||
               (tok == Tokens::GreaterThanOrEqual) ||
               (tok == Tokens::LessThan) || (tok == Tokens::LessThanOrEqual);
    }
    bool m_IsLogical(Tokens tok) {
        return (tok == Tokens::KeywordAnd) || m_IsCompareToken(tok) ||
               (tok == Tokens::KeywordOr);
    }

    ptr_t m_ParseExpr() {
        ptr_t out = m_ParseTerm();
        while (not_eof() && (m_Get().token == Tokens::Plus ||
                             m_Get().token == Tokens::Minus)) {
            if (m_Get().token == Tokens::Plus) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Plus, out,
                                                     m_ParseTerm());
            } else if (m_Get().token == Tokens::Minus) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Minus, out,
                                                     m_ParseTerm());
            }
        }
        return out;
    }
    ptr_t m_ParseTerm() {
        ptr_t out = m_ParseSu();
        while (not_eof() && (m_Get().token == Tokens::Mult ||
                             m_Get().token == Tokens::Div)) {
            if (m_Get().token == Tokens::Mult) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Mult, out,
                                                     m_ParseExpr());
            } else if (m_Get().token == Tokens::Div) {
                m_Advance();
                out =
                    std::make_shared<BinaryOpExpr>(Op::Div, out, m_ParseExpr());
            }
        }
        return out;
    }
    ptr_t m_ParseSu() {
        ptr_t out = m_ParseLogical();
        while (not_eof() &&
               (m_Get().token == Tokens::Pow || m_Get().token == Tokens::Mod)) {
            if (m_Get().token == Tokens::Pow) {
                m_Advance();
                out =
                    std::make_shared<BinaryOpExpr>(Op::Pow, out, m_ParseExpr());
            } else if (m_Get().token == Tokens::Mod) {
                m_Advance();
                out =
                    std::make_shared<BinaryOpExpr>(Op::Mod, out, m_ParseExpr());
            }
        }
        return out;
    }
    ptr_t m_ParseLogical() {
        ptr_t out = m_ParseFactor();
        while (not_eof() && (m_Get().token == Tokens::KeywordIf ||
                             m_Get().token == Tokens::KeywordElse ||
                             m_IsLogical(m_Get().token))) {
            if (m_Get().token == Tokens::KeywordIf) {
                m_Advance();
                auto stmt1 = m_ParseExpr();
                std::shared_ptr<Expr> stmt2 = nullptr;
                if (m_Get().token == Tokens::KeywordElse) stmt2 = m_ParseExpr();
                out = std::make_shared<IfExpr>(out, stmt1, stmt2);
            } else if (m_Get().token == Tokens::GreaterThan) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::Greater, out,
                                                    m_ParseExpr());
            } else if (m_Get().token == Tokens::GreaterThanOrEqual) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::GreaterOrEqual, out,
                                                    m_ParseExpr());
            } else if (m_Get().token == Tokens::LessThan) {
                m_Advance();
                out =
                    std::make_shared<Comparaison>(Op::Less, out, m_ParseExpr());
            } else if (m_Get().token == Tokens::LessThanOrEqual) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::LessOrEqual, out,
                                                    m_ParseExpr());
            } else if (m_Get().token == Tokens::KeywordAnd) {
                m_Advance();
                out = std::make_shared<LogicalExpr>(Op::LogicalAnd, out,
                                                    m_ParseExpr());
            } else if (m_Get().token == Tokens::KeywordOr) {
                m_Advance();
                out = std::make_shared<LogicalExpr>(Op::LogicalOr, out,
                                                    m_ParseExpr());
            } else if (m_Get().token == Tokens::Equals) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::Equals, out,
                                                    m_ParseExpr());
            }
        }
        return out;
    }
    ptr_t m_ParseFactor() {
        TokenHandler tok = m_Get();
        if (tok.token == Tokens::Lparen) {
            m_ParensCount++;
            m_Advance();
            if (not_eof()) {
                ptr_t out = m_ParseExpr();
                if (m_Get().token != Tokens::Rparen) {
                    m_Err();
                }
                m_Advance();
                return out;
            }
            m_Err();
        } else if (tok.token == Tokens::Digit) {
            if (not_eof()) {
                return std::make_shared<Number>(std::stod(m_GetDigit()));
            } else {
                m_Advance();
                return std::make_shared<Number>(std::stod(tok.value));
            }
        } else if (tok.token == Tokens::Plus) {
            if (not_eof() && m_Pos > 0) {
                // we don't want syntaxes such as +5 to be valid
                m_Advance();
                return std::make_shared<BinaryOpExpr>(Op::Plus, m_ParseExpr(),
                                                      nullptr);
            } else {
                m_Advance();
                m_Err();
            }
        } else if (tok.token == Tokens::Minus) {
            if (not_eof()) {
                m_Advance();
                if (m_Get().token == Tokens::Lparen ||
                    m_Get().token == Tokens::Identifier ||
                    m_Get().token == Tokens::Digit ||
                    m_Get().token == Tokens::Boolean) {
                    // m_Advance();
                    return std::make_shared<NegativeExpr>(m_ParseExpr());
                    // much easier to handle expressions like `5-(-(-(-5)))`
                } else {
                    return std::make_shared<BinaryOpExpr>(
                        Op::Minus, m_ParseExpr(), nullptr);
                }
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Identifier) {
            if (m_Peek().token == Tokens::Assign) {
                m_Advance(2);  // skip the '='
                std::string name = tok.value;
                ptr_t body = m_ParseIdentAssign();
                return std::make_shared<UserDefinedIdentifier>(name, body);
            } else if (m_Peek().token == Tokens::Lparen) {
                m_Advance(2);  // skip the '('
                return m_ParseFunctionDefOrCall(tok);
            } else {
                m_Advance();
                return std::make_shared<Identifier>(tok.value);
            }
        } else if (tok.token == Tokens::Rparen) {
            m_ParensCount--;
            if (m_ParensCount == 0) {
                m_Advance();
                return m_ParseExpr();
            } else {
                m_Err(
                    fmt::format("expected a closing paren ')' or expression "
                                "for '(' found '{}' instead",
                                tok.value));
            }
        } else if (tok.token == Tokens::Boolean) {
            m_Advance();
            return std::make_shared<Boolean>(tok.value);
        } else if (tok.token == Tokens::Semicolon) {
            m_Advance(2);
            return m_ParseExpr();
        } else {
            m_Err();
        }
    }
    void m_Err() { m_Err(fmt::format("Unexpexted token '{}'", m_Get().value)); }
    void m_Err(const std::string& msg) { m_ThrowErr("SyntaxError", msg); }
    void m_ThrowErr(const std::string& err, const std::string& msg) {
        this->ei.name = err;
        this->ei.err = msg;
        this->ei.linepos = m_Get().pos;
        throw ami::exceptions::BaseException(ei);
    }

   public:
    explicit Parser(const std::vector<TokenHandler>& _tok,
                    const std::string& str, const std::string& file) {
        this->ei =
            ami::exceptions::ExceptionInterface{.file = file, .src = str};
        if (_tok.size() > 0)
            m_Src = _tok;
        else
            m_ThrowErr("ParseError", "invalid input");
    }
    ptr_t parse() { return m_ParseExpr(); }
    std::vector<ptr_t> parsevec() {
        std::vector<ptr_t> exprs;
        while (not_eof()) exprs.push_back(m_ParseExpr());
        return exprs;
    };
    ami::exceptions::ExceptionInterface get_ei() const { return this->ei; }
};

}  // namespace ami
