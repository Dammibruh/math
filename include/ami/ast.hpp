#pragma once
#include <fmt/core.h>

#include <cstdio>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "lexer.hpp"

namespace ami {
enum class Op {
    Minus,
    Plus,
    Div,
    Mult,
    Pow,
    Mod,
    LogicalAnd,
    LogicalOr,
    Equals,
    Greater,
    Less,
    GreaterOrEqual,
    LessOrEqual
};
enum class AstType {
    Number,
    BinaryOp,
    Expr,
    Identifier,
    FunctionCall,
    Function,
    NullExpr,
    UserDefinedIdentifier,
    NegativeExpr,
    UserDefinedFunction,
    NotExpr,
    IfExpr,
    Boolean,
    ReturnExpr,
    SetObject,
    IntervalObject,
    Comparaison,
    LogicalExpr
};
static std::map<Op, char*> ops_str{
    {Op::Minus, "-"},        {Op::Plus, "+"},
    {Op::Div, "/"},          {Op::Mult, "*"},
    {Op::Pow, "^"},          {Op::Mod, "%"},
    {Op::LogicalAnd, "and"}, {Op::LogicalOr, "or"},
    {Op::Greater, ">"},      {Op::GreaterOrEqual, ">="},
    {Op::Less, "<"},         {Op::LessOrEqual, "<="},
    {Op::Equals, "=="}};
struct Expr {
    virtual std::string str() = 0;
    virtual AstType type() const = 0;
    virtual ~Expr() = default;
};
struct Number : public Expr {
    long double val;
    explicit Number(long double x) : val(x) {}
    std::string str() override {
        return fmt::format("<Number value=<{}>>", val);
    }
    AstType type() const override { return AstType::Number; }
    std::string to_str() {
        std::stringstream ss;
        ss << std::setprecision(15);
        ss << val;
        return ss.str();
    }
};
struct Boolean : public Expr {
    bool val;
    std::string raw_value;
    Boolean(std::string_view _val) : raw_value(_val) {
        if (_val == "true")
            val = true;
        else if (_val == "false")
            val = false;
    }
    Boolean(bool _val) : val(_val) {
        if (_val)
            raw_value = "true";
        else if (!_val)
            raw_value = "false";
    }
    AstType type() const override { return AstType::Boolean; }
    std::string str() override {
        return fmt::format("<Boolean value={}>", raw_value);
    }
};
struct Comparaison : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    Comparaison(Op o, const std::shared_ptr<Expr>& l,
                const std::shared_ptr<Expr>& r)
        : op(o), lhs(l), rhs(r) {}
    AstType type() const override { return AstType::Comparaison; }
    std::string str() override {
        return fmt::format(
            "<Comparaison left=<{left}> op=<{op}>, right=<{right}>>",
            fmt::arg("left", lhs->str()), fmt::arg("op", ops_str.at(op)),
            fmt::arg("right", rhs->str()));
    }
};
struct LogicalExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    LogicalExpr(Op o, const std::shared_ptr<Expr>& l,
                const std::shared_ptr<Expr>& r)
        : op(o), lhs(l), rhs(r) {}
    AstType type() const override { return AstType::LogicalExpr; }
    std::string str() override {
        return fmt::format("<LogicalExpr lhs=<{l}>, op=<{op}>, rhs=<{r}>>",
                           fmt::arg("l", lhs->str()), fmt::arg("r", rhs->str()),
                           fmt::arg("op", ops_str.at(op)));
    }
};
struct NegativeExpr : public Expr {
    std::shared_ptr<Expr> value;
    explicit NegativeExpr(const std::shared_ptr<Expr>& val) : value(val) {}
    std::string str() override {
        return fmt::format("<NegativeExpr value=<{}>>", value->str());
    }
    AstType type() const override { return AstType::NegativeExpr; }
};
struct NotExpr : public Expr {
    std::shared_ptr<Expr> value;
    explicit NotExpr(const std::shared_ptr<Expr>& val) : value(val) {}
    std::string str() override {
        return fmt::format("<NotExpr value=<{}>>", value->str());
    }
    AstType type() const override { return AstType::NotExpr; }
};
struct NullExpr : public Expr {
    std::string value = "null";
    NullExpr() {}
    std::string str() override { return "<NullExpr>"; }
    AstType type() const override { return AstType::NullExpr; }
};
struct BinaryOpExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    BinaryOpExpr(Op c, const std::shared_ptr<Expr>& _lhs,
                 const std::shared_ptr<Expr>& _rhs)
        : lhs(_lhs), rhs(_rhs), op(c) {}
    std::string str() override {
        std::string lhs_str = lhs != nullptr ? lhs->str() : "null";
        std::string rhs_str = rhs != nullptr ? rhs->str() : "null";
        return fmt::format("<BinaryOpExpr left=<{}>, right=<{}>>", lhs_str,
                           rhs_str);
    }
    AstType type() const override { return AstType::BinaryOp; }
};
struct Identifier : public Expr {
    std::string name;
    explicit Identifier(const std::string& name) : name(name) {}
    std::string str() override {
        return fmt::format("<Identifier name=<{}>>", name);
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
        return fmt::format("<UserDefinedIdentifier name=<{}>, value=<{}>>",
                           name, value->str());
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
        ss << "<FunctionCall name=<" << name << ">, args=<";
        if (arguments.empty()) {
            ss << "null>>";
        } else {
            for (auto& arg : arguments) {
                ss << arg->str() << ", ";
            }
            ss << ">>";
        }
        return ss.str();
    }
};
struct Function : public Expr {
    std::string name;
    std::size_t call_count;
    std::shared_ptr<Expr> body, ReturnStmt;
    std::vector<std::shared_ptr<Expr>> arguments;
    std::map<std::string, std::variant<Number, Boolean, NullExpr, std::string>>
        scope;
    Function(std::string_view name, const std::shared_ptr<Expr>& body,
             const std::vector<std::shared_ptr<Expr>>& args)
        : name(name), body(body), arguments(args), call_count(0) {}
    AstType type() const override { return AstType::Function; }
    std::string str() override {
        std::string str;
        str += ("<Function name=<" + name + ">, args=<");
        if (arguments.size() > 0) {
            for (auto& ar : arguments) str += ar->str() + ", ";
        } else {
            str += "null";
        }
        str += std::string(">, ") + "body=<" + body->str() + ">>";
        return str;
    }
};
struct IfExpr : public Expr {
    std::shared_ptr<Expr> cond, body, elsestmt;
    IfExpr(const std::shared_ptr<Expr>& cond, const std::shared_ptr<Expr>& st1,
           const std::shared_ptr<Expr>& st2)
        : cond(cond), body(st1), elsestmt(st2) {}
    AstType type() const override { return AstType::IfExpr; }
    std::string str() override {
        std::string str = fmt::format(
            "<IfExpr condition=<{cond}>, if_true=<{stmt1}>, else=<{stmt2}>>",
            fmt::arg("cond", cond->str()), fmt::arg("stmt1", body->str()),
            fmt::arg("stmt2", elsestmt != nullptr ? elsestmt->str() : "null"));
        return str;
    }
};
// later cuz me is epic
struct SetObject : public Expr {};
struct IntervalExpr : public Expr {};
}  // namespace ami
