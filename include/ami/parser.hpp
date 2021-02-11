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
#include "lexer.hpp"
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
    bool is_in_func_args{};
    // to disable syntax checking for nunbers in funtion's args

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
    bool m_IsAnOp(TokenHandler tok) {
        return tok.is(Tokens::Mod, Tokens::Div, Tokens::Mult, Tokens::Plus,
                      Tokens::Minus, Tokens::Pow);
    }
    bool m_IsValidAfterNumber(TokenHandler tok) {
        return m_IsAnOp(tok) ||
               tok.is(Tokens::Lparen, Tokens::Rparen, Tokens::Digit,
                      Tokens::Delim, Tokens::Edelim, Tokens::KeywordElse,
                      Tokens::Dot, Tokens::Semicolon, Tokens::Lcbracket,
                      Tokens::Rcbracket, Tokens::KeywordIn);
    }
    std::vector<ptr_t> m_ParseSplitedInput(Tokens end, Tokens delim,
                                           const std::string& delimstr,
                                           const std::string& msg) {
        /*
         * end: represents the end token for the inpuy
         * delim: delim to separate each input
         * delimstr: string version of the delim
         * msg: wtf are we parsing ? useful for errors
         * */
        std::vector<ptr_t> out{};
        if (m_Get().isNot(end)) {
            do {
                if (m_Get().is(delim)) {
                    if (out.size() > 0 && m_Peek().isNot(delim) && not_eof())
                        m_Advance();
                    else
                        m_Err();
                } else if (!not_eof()) {
                    m_ThrowErr("ParseError",
                               fmt::format("EOF while parsing {}", msg));
                } else {
                    out.push_back(m_ParseComp());
                }
            } while (m_Get().isNot(Tokens::Rparen));
        }
        if (m_Get().isNot(end)) {
            m_ThrowErr("SyntaxError",
                       fmt::format("expected '{}' after {}", delimstr, msg));
        }
        m_Advance();
        return out;
    }
    std::vector<ptr_t> m_ParseFunctionArgs() {
        is_in_func_args = true;
        std::vector<ptr_t> args = m_ParseSplitedInput(
            Tokens::Rparen, Tokens::Comma, ",", "function arguments");
        is_in_func_args = false;
        return args;
    }
    std::vector<ptr_t> m_ParseFunctionDefArgs() {
        std::vector<ptr_t> args{};
        is_in_func_args = true;
        if (m_Get().isNot(Tokens::Rparen)) {
            do {
                if (m_Get().is(Tokens::Comma)) {
                    if (args.size() > 0 && m_Peek().isNot(Tokens::Rparen) &&
                        not_eof())
                        m_Advance();
                    else
                        m_Err();
                } else if (!not_eof()) {
                    m_ThrowErr("ParseError",
                               "EOF while parsing function arguments ");
                } else {
                    ptr_t temp = m_ParseComp();
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
            } while (m_Get().isNot(Tokens::Rparen));
        }
        if (m_Get().isNot(Tokens::Rparen)) {
            m_ThrowErr("SyntaxError", "expected ')' after arguments list");
        }
        is_in_func_args = false;
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
            return t.is(Tokens::Rparen);
        });
        auto get_fdef = std::find_if(src.begin(), src.end(), [](auto t) {
            return t.is(Tokens::FunctionDef);
        });
        bool contains_fdef = get_fdef != src.end();
        bool invalid_fdef = get_fdef < get_rparen;
        // to prevent somth like f(->) from being a valid syntax
        std::vector<ptr_t> args;
        if (contains_fdef && !invalid_fdef) {
            args = m_ParseFunctionDefArgs();
        } else if (!contains_fdef) {
            args = m_ParseFunctionArgs();
        } else {
            // m_Err();
        }
        m_Advance();
        if (contains_fdef) {
            ptr_t body = m_ParseComp();
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
    ptr_t m_ParseInterval(TokenHandler tok) {
        bool left_is_strict = tok.is(Tokens::Rcbracket);
        m_Advance();
        if (m_Get().isNot(Tokens::Rcbracket, Tokens::Lcbracket) && not_eof()) {
            ptr_t left_ = m_ParseFactor();  // since only numbers are valid
            if (m_Get().is(Tokens::Semicolon)) {
                m_Advance();
                ptr_t right_ = m_ParseFactor();
                bool right_is_strict = m_Get().is(Tokens::Rcbracket);
                m_Advance();
                return std::make_shared<IntervalExpr>(
                    IntervalHandler(left_, left_is_strict),
                    IntervalHandler(right_, right_is_strict));
            } else {
                m_Err(
                    fmt::format("expected ';' for interval found '{}' instead",
                                m_Get().value));
            }
        } else {
            m_Err();
        }
    }
    ptr_t m_ParseIdentAssign() {
        ptr_t value = m_ParseComp();
        while (not_eof() && m_Get().isNot(Tokens::Semicolon)) {
            m_Advance();
            value = m_ParseComp();
        }
        return value;
    }
    bool m_IsValidPunc(TokenHandler tok) {
        return tok.is(Tokens::Digit, Tokens::Dot, Tokens::Delim, Tokens::Edelim,
                      Tokens::Minus, Tokens::Semicolon);
    }
    bool m_IsBreakable(TokenHandler tok) {
        return tok.is(Tokens::Semicolon, Tokens::KeywordIn) ||
               m_IsCompareToken(tok) || m_IsLogical(tok);
    }
    std::string m_GetDigit() {
        std::string temp{};
        bool is_decimal{};
        bool contains_e{};
        while (not_eof() && m_IsValidPunc(m_Get())) {
            if (m_Get().is(Tokens::Digit)) {
                temp += m_Get().value;
            } else if (m_Get().is(Tokens::Dot)) {
                if (is_decimal || !m_IsValidPunc(m_Peek())) {
                    m_Err();
                } else {
                    temp += m_Get().value;
                    is_decimal = true;
                }
            } else if (m_Get().is(Tokens::Edelim)) {
                if (contains_e) {
                    m_Err();
                } else {
                    temp += m_Get().value;
                    contains_e = true;
                }
            } else if (m_Get().is(Tokens::Minus)) {
                if (contains_e && m_Prev().is(Tokens::Edelim)) {
                    temp += m_Get().value;
                    // if the digit is 1e-10 parse the e-10 then break cuz we
                    // don't want to ignore the minus operator
                } else {
                    break;
                }
            } else if (m_IsBreakable(m_Get())) {
                break;
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
    bool m_IsCompareToken(TokenHandler tok) {
        return tok.is(Tokens::GreaterThan, Tokens::Equals,
                      Tokens::GreaterThanOrEqual, Tokens::LessThan,
                      Tokens::LessThanOrEqual);
    }
    bool m_IsLogical(TokenHandler tok) {
        return tok.is(Tokens::KeywordAnd, Tokens::KeywordOr);
    }
    ptr_t m_ParseComp() {
        ptr_t out = m_ParseExpr();
        while (not_eof() && (m_IsCompareToken(m_Get()))) {
            if (m_Get().is(Tokens::GreaterThan)) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::Greater, out,
                                                    m_ParseExpr());
            } else if (m_Get().is(Tokens::GreaterThanOrEqual)) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::GreaterOrEqual, out,
                                                    m_ParseExpr());
            } else if (m_Get().is(Tokens::LessThan)) {
                m_Advance();
                out =
                    std::make_shared<Comparaison>(Op::Less, out, m_ParseExpr());
            } else if (m_Get().is(Tokens::LessThanOrEqual)) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::LessOrEqual, out,
                                                    m_ParseExpr());
            } else if (m_Get().is(Tokens::Equals)) {
                m_Advance();
                out = std::make_shared<Comparaison>(Op::Equals, out,
                                                    m_ParseExpr());
            }
        }
        return out;
    }
    ptr_t m_ParseExpr() {
        ptr_t out = m_ParseTerm();
        while (not_eof() &&
               m_Get().is(Tokens::Plus, Tokens::Minus, Tokens::KeywordIn)) {
            if (m_Get().is(Tokens::Plus)) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Plus, out,
                                                     m_ParseTerm());
            } else if (m_Get().is(Tokens::Minus)) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Minus, out,
                                                     m_ParseTerm());
            } else if (m_Get().is(Tokens::KeywordIn)) {
                m_Advance();
                out = std::make_shared<IntervalIn>(out, m_ParseFactor());
            }
        }
        return out;
    }
    ptr_t m_ParseTerm() {
        ptr_t out = m_ParseSu();
        while (not_eof() && m_Get().is(Tokens::Mult, Tokens::Div)) {
            if (m_Get().is(Tokens::Mult)) {
                m_Advance();
                out =
                    std::make_shared<BinaryOpExpr>(Op::Mult, out, m_ParseSu());
            } else if (m_Get().is(Tokens::Div)) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Div, out, m_ParseSu());
            }
        }
        return out;
    }
    ptr_t m_ParseSu() {
        ptr_t out = m_ParseLogical();
        while (not_eof() && m_Get().is(Tokens::Pow, Tokens::Mod)) {
            if (m_Get().is(Tokens::Pow)) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Pow, out,
                                                     m_ParseLogical());
            } else if (m_Get().is(Tokens::Mod)) {
                m_Advance();
                out = std::make_shared<BinaryOpExpr>(Op::Mod, out,
                                                     m_ParseLogical());
            }
        }
        return out;
    }
    ptr_t m_ParseLogical() {
        ptr_t out = m_ParseFactor();
        while (not_eof() && (m_IsLogical(m_Get()))) {
            if (m_Get().is(Tokens::KeywordAnd)) {
                m_Advance();
                out = std::make_shared<LogicalExpr>(Op::LogicalAnd, out,
                                                    m_ParseFactor());
            } else if (m_Get().is(Tokens::KeywordOr)) {
                m_Advance();
                out = std::make_shared<LogicalExpr>(Op::LogicalOr, out,
                                                    m_ParseFactor());
            } else if (m_Get().is(Tokens::KeywordNot)) {
                out = std::make_shared<NotExpr>(out);
            }
        }
        return out;
    }
    ptr_t m_ParseFactor() {
        TokenHandler tok = m_Get();
        if (tok.is(Tokens::Lparen)) {
            m_Advance();
            if (not_eof()) {
                ptr_t out = m_ParseComp();
                if (m_Get().isNot(Tokens::Rparen)) {
                    m_Err();
                }
                m_Advance();
                return out;
            }
            m_Err();
        } else if (tok.is(Tokens::Digit)) {
            if (not_eof() && !is_in_func_args) {
                if (m_IsValidAfterNumber(m_Peek()) ||
                    m_IsCompareToken(m_Peek()) || m_IsLogical(m_Peek())) {
                    return std::make_shared<Number>(std::stold(m_GetDigit()));
                } else {
                    m_Err();
                }
            } else {
                m_Advance();
                return std::make_shared<Number>(std::stod(tok.value));
            }
        } else if (tok.is(Tokens::Plus)) {
            if (not_eof() && m_Pos > 0) {
                // we don't want syntaxes such as +5 to be valid
                m_Advance();
                return std::make_shared<BinaryOpExpr>(Op::Plus, m_ParseComp(),
                                                      nullptr);
            } else {
                m_Advance();
                m_Err();
            }
        } else if (tok.is(Tokens::Minus)) {
            if (not_eof()) {
                m_Advance();
                if (m_Get().is(Tokens::Lparen, Tokens::Identifier,
                               Tokens::Digit, Tokens::Boolean)) {
                    // m_Advance();
                    return std::make_shared<NegativeExpr>(m_ParseFactor());
                    // much easier to handle expressions like `5-(-(-(-5)))`
                } else {
                    return std::make_shared<BinaryOpExpr>(
                        Op::Minus, m_ParseComp(), nullptr);
                }
            } else {
                m_Err();
            }
        } else if (tok.is(Tokens::Identifier)) {
            if (m_Peek().is(Tokens::Assign)) {
                m_Advance(2);  // skip the '='
                std::string name = tok.value;
                ptr_t body = m_ParseIdentAssign();
                return std::make_shared<UserDefinedIdentifier>(name, body);
            } else if (m_Peek().is(Tokens::Lparen)) {
                m_Advance(2);  // skip the '('
                return m_ParseFunctionDefOrCall(tok);
            } else {
                m_Advance();
                return std::make_shared<Identifier>(tok.value);
            }
        } else if (tok.is(Tokens::Boolean)) {
            m_Advance();
            return std::make_shared<Boolean>(tok.value);
        } else if (tok.is(Tokens::Semicolon)) {
            m_Advance(2);
            return m_ParseComp();
        } else if (tok.is(Tokens::KeywordIf)) {
            m_Advance();
            if (m_Get().is(Tokens::Lparen)) {
                ptr_t cond = m_ParseComp();
                m_Advance(-1);
                if (m_Get().isNot(Tokens::Rparen)) {
                    m_Err(fmt::format(
                        "expected a closing ')' for 'if' found '{}'",
                        m_Get().value));
                } else {
                    m_Advance();
                    if (!not_eof())
                        m_Err("expected an expression after 'if' statement");
                    ptr_t stmt1 = m_ParseComp();
                    ptr_t stmt2 = nullptr;
                    if (m_Get().is(Tokens::KeywordElse)) {
                        if (!not_eof())
                            m_Err(
                                "expected an expression after 'else' "
                                "statement");
                        m_Advance();
                        stmt2 = m_ParseComp();
                    }
                    return std::make_shared<IfExpr>(cond, stmt1, stmt2);
                }
            } else {
                m_Err(
                    fmt::format("expected a '(' after keyword 'if' found '{}'",
                                m_Get().value));
            }
        } else if (tok.is(Tokens::KeywordNull)) {
            m_Advance();
            return std::make_shared<NullExpr>();  // literally just  a null
        } else if (tok.is(Tokens::KeywordNot)) {
            if (not_eof()) {
                m_Advance();
                return std::make_shared<NotExpr>(m_ParseComp());
            } else {
                m_Err();
            }
        } else if (tok.is(Tokens::Lcbracket, Tokens::Rcbracket)) {
            return m_ParseInterval(tok);
        } else {
            m_Err();
        }
    }
    void m_Err() { m_Err("invalid syntax"); }
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
    ptr_t parse() { return m_ParseComp(); }
    std::vector<ptr_t> parsevec() {
        std::vector<ptr_t> exprs;
        while (not_eof()) exprs.push_back(m_ParseComp());
        return exprs;
    }
    ami::exceptions::ExceptionInterface get_ei() const { return this->ei; }
};

}  // namespace ami
