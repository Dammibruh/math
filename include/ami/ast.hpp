#pragma once
#include <cstdio>
#include <memory>
#include <sstream>

#include "lexer.hpp"

namespace ami {
enum class Op { Minus, Plus, Div, Mult, Pow, Mod };
enum class AstType {
    Number,
    BinaryOp,
    Expr,
    Identifier,
    FunctionCall,
    Function,
    UserDefinedIdentifier,
    UserDefinedFunction
};
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
        std::string out;
        out += ("<NUMBER:" + std::to_string(val) + '>');
        return out;
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
        ss << "<Identifier name={" << name << "}>";
        return ss.str();
    }
    AstType type() const override { return AstType::Identifier; }
};
struct UserDefinedIdentifier : public Expr {
    std::string name;
    std::unique_ptr<Expr> value;
    UserDefinedIdentifier(const std::string& name, std::unique_ptr<Expr> val)
        : name(name), value(std::move(val)) {}
    std::string str() override {
        std::stringstream ss;
        ss << "<UserDefinedIdentifier name={" << name << "}, value={"
           << value->str() << "}>";
        return ss.str();
    }
    AstType type() const override { return AstType::UserDefinedIdentifier; }
};
struct FunctionCall : public Expr {
    std::vector<std::unique_ptr<Expr>> arguments;
    std::string name;
    FunctionCall(std::string_view name, std::vector<std::unique_ptr<Expr>> args)
        : name(name), arguments(std::move(args)) {}
    AstType type() const override { return AstType::FunctionCall; }
    std::string str() override {
        std::stringstream ss;
        ss << "<FunctionCall name={" << name << "}, args={";
        if (arguments.empty()) {
            ss << "null}>";
        } else {
            for (auto& arg : arguments) {
                ss << arg->str() << ", ";
            }
            ss << "}>";
        }
        return ss.str();
    }
};
struct Function : public Expr {
    std::vector<std::unique_ptr<Expr>> arguments;
    std::string name, body;
    Function(std::string_view name, std::string_view body,
             std::vector<std::unique_ptr<Expr>> args)
        : name(name), body(body), arguments(std::move(args)) {}
    AstType type() const override { return AstType::Function; }
    std::string str() override {
        std::string str;
        str += ("<Function name={" + name + "}>");
        return str;
    }
};
}  // namespace ami
