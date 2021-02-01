#pragma once
#include <cstdio>
#include <memory>
#include <sstream>

#include "lexer.hpp"

enum class Op { Minus, Plus, Div, Mult, Pow, Mod };
enum class AstType { Number, BinaryOp, Expr, Identifier, FunctionCall };
std::map<Op, char> ops_str{{Op::Minus, '-'}, {Op::Plus, '+'}, {Op::Div, '/'},
                           {Op::Mult, '*'},  {Op::Pow, '^'},  {Op::Mod, '%'}};
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
        std::sprintf(ch, "<NUMBER:%f>", val);
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
struct Identifier : public Expr {
    std::string name;
    Identifier(const std::string& name) : name(name) {}
    std::string str() override {
        std::stringstream ss;
        ss << '<' << name << '>';
        return ss.str();
    }
    AstType type() const override { return AstType::Identifier; }
};
