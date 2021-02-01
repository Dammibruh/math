#pragma once
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "ast.hpp"
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
    bool m_IsIdent(const std::string& str) {
        std::string::size_type count = 0;
        for (auto& c : str)
            if (std::isalpha(c) || c == '_') count++;
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
            if (m_Get().token != Tokens::Rparen) {
                m_Err();
            }
            m_Advance();
            return std::move(out);
        } else if (tok.token == Tokens::Plus) {
            // we don't want a seg fault while trying to parse an operator at
            // the beginning eg: +5
            if (m_Pos > 0) {
                m_Advance();
                return std::make_unique<BinaryOpExpr>(
                    Op::Plus, std::move(m_ParseFactor()), nullptr);
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Minus) {
            // we don't want a seg fault while trying to parse an operator at
            // the beginning eg: -5 since negative numbers are handled by the
            // lexer
            if (m_Pos > 0) {
                m_Advance();
                return std::make_unique<BinaryOpExpr>(
                    Op::Minus, std::move(m_ParseFactor()), nullptr);
            } else {
                m_Err();
            }
        } else if (tok.token == Tokens::Identifier) {
            m_Advance();
            return std::make_unique<Identifier>(tok.value);
        } else if (tok.token == Tokens::Digit) {
            try {
                m_Advance();
                return std::make_unique<Number>(std::stod(tok.value));
            } catch (...) {
                throw std::runtime_error("unkown expression");
            }
        }
        m_Err();
    }
    void m_Err() {
        std::stringstream ss;
        ss << "Syntax Error at " << m_Pos << " invalid expression \""
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

