#pragma once
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ast.hpp"
namespace ami {
class Parser {
    using ptr_t = std::shared_ptr<Expr>;
    std::vector<TokenHandler> m_Src;
    std::size_t m_Pos = 0;
    bool is_in_func_args = false;

    TokenHandler m_Get() { return m_Src[m_Pos]; }
    bool not_eof() { return m_Pos < m_Src.size(); }
    bool m_IsDigit(const std::string& str) {
        for (auto& c : str)
            if (!std::isdigit(c) && c != '-') return false;
        return true;
    }
    std::string m_ParseFunctionBody() {
        std::string body{};
        while (not_eof() || m_Get().token == Tokens::Semicolon) {
            body += m_Get().value;
            m_Advance();
            if (m_Get().token == Tokens::Semicolon) {
                m_Advance();
                return body;
            }
        }
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
                        std::to_string(m_Pos));
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
    ptr_t m_ParseIdentAssign() {
        ptr_t value = m_ParseExpr();
        while (not_eof() && m_Get().token != Tokens::Semicolon) {
            m_Advance();
            value = m_ParseExpr();
        }
        return value;
    }
    std::string m_GetDigit() {
        std::string temp{};
        bool is_decimal{};
        bool contains_e{};
        while (not_eof() && (m_Get().token == Tokens::Digit ||
                             m_Get().token == Tokens::Dot ||
                             m_Get().token == Tokens::Delim ||
                             m_Get().token == Tokens::Edelim)) {
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
    TokenHandler m_Peek() { return m_Src[m_Pos + 1]; }
    bool m_IsIdent(const std::string& str) {
        std::string::size_type count = 0;
        for (auto& c : str)
            if (std::isalpha(c) || c == '_') count++;
        return count == str.size();
    }
    void m_Advance(std::size_t x = 1) {
        if (m_Pos >= 0 && m_Pos < m_Src.size())
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
                return std::make_shared<BinaryOpExpr>(Op::Minus,
                                                      m_ParseFactor(), nullptr);
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
                is_in_func_args = true;
                std::vector<ptr_t> args = m_ParseFunctionArgs();
                is_in_func_args = false;
                return std::make_shared<FunctionCall>(name, args);
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
            }
            m_Err();
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
        while (not_eof()) {
            out = m_ParseExpr();
        }
        return out;
    }
};

}  // namespace ami
