#pragma once
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

#include "ast.hpp"
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
    bool is_in_func_args = false;

    TokenHandler m_Get() {
        return m_Src.at(m_Pos >= m_Src.size() ? m_Src.size() - 1 : m_Pos);
    }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_IsDigit(const std::string& str) {
        for (auto& c : str)
            if (!std::isdigit(c) && c != '-') return false;
        return true;
    }
    ptr_t m_ParseFunctionBody() {
        ptr_t body = m_ParseExpr();
        while (not_eof() && m_Get().token != Tokens::Semicolon) {
            m_Advance();
            body = m_ParseExpr();
        }
        return body;
    }
    std::vector<ptr_t> m_ParseFunctionArgs() {
        std::vector<ptr_t> args{};
        if (m_Get().token != Tokens::Rparen) {
            do {
                if (m_Get().token == Tokens::Comma) {
                    m_Advance();
                } else if (!not_eof()) {
                    throw std::runtime_error(
                        "EOF while parsing function arguments " +
                        m_Get().value);
                } else {
                    args.push_back(m_ParseExpr());
                }
            } while (m_Get().token != Tokens::Rparen);
        }
        if (m_Get().token != Tokens::Rparen) {
            throw std::runtime_error("expected ')' after arguments list");
        }
        m_Advance();
        return args;
    }
    std::vector<ptr_t> m_ParseFunctionDefArgs() {
        std::vector<ptr_t> args{};
        if (m_Get().token != Tokens::Rparen) {
            do {
                if (m_Get().token == Tokens::Comma) {
                    m_Advance();
                } else if (!not_eof()) {
                    throw std::runtime_error(
                        "EOF while parsing function arguments " +
                        m_Get().value);
                } else {
                    ptr_t temp = m_ParseExpr();
                    if (temp->type() == AstType::Identifier) {
                        args.push_back(temp);
                    } else {
                        throw std::runtime_error(
                            "expected identifier in function's arguments");
                    }
                }
            } while (m_Get().token != Tokens::Rparen);
        }
        if (m_Get().token != Tokens::Rparen) {
            throw std::runtime_error("expected ')' after arguments list");
        }
        m_Advance();
        return args;
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
                tok.token == Tokens::Delim || tok.token == Tokens::Edelim);
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
                out =
                    std::make_shared<BinaryOpExpr>(Op::Mult, out, m_ParseSu());
            } else if (m_Get().token == Tokens::Div) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Div, out, m_ParseSu());
            }
        }
        return out;
    }
    ptr_t m_ParseSu() {
        ptr_t out = m_ParseFactor();
        while (not_eof() &&
               (m_Get().token == Tokens::Pow || m_Get().token == Tokens::Mod)) {
            if (m_Get().token == Tokens::Pow) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Pow, out,
                                                     m_ParseFactor());
            } else if (m_Get().token == Tokens::Mod) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Mod, out,
                                                     m_ParseFactor());
            }
        }
        return out;
    }
    ptr_t m_ParseFactor() {
        TokenHandler tok = m_Get();
        if (tok.token == Tokens::Lparen) {
            m_Advance();
            ptr_t out = m_ParseExpr();
            if (m_Get().token != Tokens::Rparen && !is_in_func_args) {
                m_Err();
            }
            m_Advance();
            return out;
        } else if (tok.token == Tokens::Digit) {
            if (m_IsDigit(tok.value) && not_eof()) {
                return std::make_shared<Number>(std::stod(m_GetDigit()));
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Plus) {
            if (m_Pos > 0 && not_eof()) {
                m_Advance();
                return std::make_shared<BinaryOpExpr>(Op::Plus, m_ParseFactor(),
                                                      nullptr);
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Minus) {
            if (m_Pos > 0 && not_eof()) {
                // we don't want to consider negative numbers as an
                // operation
                m_Advance();
                if (m_Get().token == Tokens::Lparen) {
                    m_Advance();
                    return std::make_shared<NegativeExpr>(m_ParseExpr());
                    // much easier to handle expressions like `5-(-(-(-5)))`
                } else {
                    return std::make_shared<BinaryOpExpr>(
                        Op::Minus, m_ParseFactor(), nullptr);
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
                std::string name = tok.value;
                std::vector<TokenHandler> src(m_Src.begin() + m_Pos,
                                              m_Src.end());
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
                auto get_rparen = std::find_if(
                    src.begin(), src.end(),
                    [](auto t) { return t.token == Tokens::Rparen; });
                auto get_assign = std::find_if(
                    src.begin(), src.end(),
                    [](auto t) { return t.token == Tokens::Assign; });
                bool contains_assign = get_assign != src.end();
                bool invalid_assign = get_assign < get_rparen;
                is_in_func_args = true;
                std::vector<ptr_t> args;
                if (contains_assign && !invalid_assign) {
                    args = m_ParseFunctionDefArgs();
                } else if (!contains_assign) {
                    args = m_ParseFunctionArgs();
                } else {
                    m_Err();
                }
                m_Advance();
                is_in_func_args = false;
                if (contains_assign) {
                    ptr_t body = m_ParseFunctionBody();
                    return std::make_shared<Function>(name, body, args);
                } else {
                    return std::make_shared<FunctionCall>(name, args);
                }
            } else {
                m_Advance();
                return std::make_shared<Identifier>(tok.value);
            }
        } else if (tok.token == Tokens::Comma) {
            if (!is_in_func_args) {
                m_Err();
            }
        } else if (tok.token == Tokens::Rparen) {
            if (is_in_func_args) {
                m_Advance();
                is_in_func_args = false;
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Semicolon) {
            m_Advance(2);
            return m_ParseExpr();
        } else {
            m_Err();
        }
    }
    void m_Err() { m_ThrowErr("Syntax Error"); }
    void m_ThrowErr(std::string_view msg) {
        std::stringstream ss;
        ss << msg << " at " << m_Pos << " invalid token \"" << m_Get().value
           << '"';
        throw std::runtime_error(ss.str());
    }

   public:
    explicit Parser(const std::vector<TokenHandler>& _tok) {
        if (_tok.size() > 0)
            m_Src = _tok;
        else
            throw std::runtime_error("invalid input");
    }
    ptr_t parse() {
        ptr_t out;
        return m_ParseExpr();
    }
};

}  // namespace ami
