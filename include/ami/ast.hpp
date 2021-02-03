#pragma once
#include <cstdio>
#include <map>
#include <memory>
#include <sstream>
#include <string>

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
static std::map<Op, char> ops_str{{Op::Minus, '-'}, {Op::Plus, '+'},
                                  {Op::Div, '/'},   {Op::Mult, '*'},
                                  {Op::Pow, '^'},   {Op::Mod, '%'}};
struct Expr {
    virtual std::string str() = 0;
    virtual AstType type() const { return AstType::Expr; }
    virtual ~Expr() = default;
};
struct Number : Expr {
    double val;
    explicit Number(double x) : val(x) {}
    std::string str() override {
        std::string out;
        out += ("<NUMBER:" + std::to_string(val) + '>');
        return out;
    }
    AstType type() const override { return AstType::Number; }
};
struct BinaryOpExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    BinaryOpExpr(Op c, const std::shared_ptr<Expr>& _lhs,
                 const std::shared_ptr<Expr>& _rhs)
        : lhs(_lhs), rhs(_rhs), op(c) {}
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
    std::shared_ptr<Expr> value;
    UserDefinedIdentifier(const std::string& name,
                          const std::shared_ptr<Expr>& val)
        : name(name), value(val) {}
    std::string str() override {
        std::stringstream ss;
        ss << "<UserDefinedIdentifier name={" << name << "}, value={"
           << value->str() << "}>";
        return ss.str();
    }
    AstType type() const override { return AstType::UserDefinedIdentifier; }
};
struct FunctionCall : public Expr {
    std::string name;
    std::vector<std::shared_ptr<Expr>> arguments;
    FunctionCall(std::string_view name,
                 const std::vector<std::shared_ptr<Expr>>& args)
        : name(name), arguments(args) {}
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
    std::string name, body;
    std::vector<std::shared_ptr<Expr>> arguments;
    Function(std::string_view name, std::string_view body,
             const std::vector<std::shared_ptr<Expr>>& args)
        : name(name), body(body), arguments(args) {}
    AstType type() const override { return AstType::Function; }
    std::string str() override {
        std::string str;
        str += ("<Function name={" + name + "}>");
        return str;
    }
};
}  // namespace ami
