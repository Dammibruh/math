#pragma once
#include <fmt/core.h>

#include <compare>
#include <cstdio>
#include <iomanip>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <utility>
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
    Interval,
    IntervalIn,
    IntervalUnion,
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
    std::string to_str(int l = 15) {
        std::stringstream ss;
        ss << std::setprecision(l);
        ss << val;
        return ss.str();
    }
    std::strong_ordering operator<=>(const Number& oth) {
        if (val > oth.val) {
            return std::strong_ordering::greater;
        } else if (val < oth.val) {
            return std::strong_ordering::less;
        } else if (val == oth.val) {
            return std::strong_ordering::equal;
        }
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
    std::strong_ordering operator<=>(const Boolean& oth) {
        if (val > oth.val) {
            return std::strong_ordering::greater;
        } else if (val < oth.val) {
            return std::strong_ordering::less;
        } else if (val == oth.val) {
            return std::strong_ordering::equal;
        }
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
struct IntervalHandler {
    std::shared_ptr<Expr> value;
    bool strict;
    IntervalHandler(const std::shared_ptr<Expr>& v, bool s = false)
        : value(v), strict(s) {}
};
struct IntervalExpr : public Expr {
    IntervalHandler min, max;
    IntervalExpr(IntervalHandler n, IntervalHandler u) : min(n), max(u) {}
    std::string str() override {
        return fmt::format(
            "<Interval min=<value: {}, strict {}>, max=<value: {}, strict: "
            "{}>>",
            min.value->str(), min.strict, max.value->str(), max.strict);
    }
    IntervalExpr(const IntervalExpr& oth) = default;
    AstType type() const override { return AstType::Interval; }
    std::string to_str() {
        char lft = min.strict ? ']' : '[';
        char rgt = max.strict ? '[' : ']';
        return fmt::format(
            "{}{}; {}{}", lft, static_cast<Number*>(min.value.get())->to_str(),
            static_cast<Number*>(max.value.get())->to_str(), rgt);
    }
};
struct IntervalIn : public Expr {
    std::shared_ptr<Expr> number;
    std::shared_ptr<Expr> inter;  // no worries the cool guy interpreter will
                                  // handle this parser is just for syntax
    IntervalIn(const std::shared_ptr<Expr>& h,
               const std::shared_ptr<Expr>& inter)
        : number(h), inter(inter) {}
    IntervalIn(const IntervalIn& oth) = default;
    std::string str() override {
        return fmt::format("<IntervalIn number=<{}>, interval=<{}>>",
                           number->str(), inter->str());
    }
    AstType type() const override { return AstType::IntervalIn; }
};
struct IntervalUnion : public Expr {
    std::shared_ptr<Expr> left_interval, right_interval;
    IntervalUnion(const std::shared_ptr<Expr>& h,
                  const std::shared_ptr<Expr>& inter)
        : left_interval(h), right_interval(inter) {}
    IntervalUnion(const IntervalUnion& oth) = default;
    AstType type() const override { return AstType::IntervalUnion; }
    std::string str() override {
        return fmt::format("<IntervalUnion left=<{}>, right=<{}>>",
                           left_interval->str(), right_interval->str());
    }
    std::string to_str() {
        if ((left_interval->type() == AstType::Interval) &&
            right_interval->type() == AstType::Interval) {
            return fmt::format(
                "{} union {}",
                static_cast<IntervalExpr*>(left_interval.get())->to_str(),
                static_cast<IntervalExpr*>(right_interval.get())->to_str());
        } else if ((left_interval->type() == AstType::IntervalUnion) &&
                   right_interval->type() == AstType::Interval) {
            return fmt::format(
                "{} union {}",
                static_cast<IntervalUnion*>(left_interval.get())->to_str(),
                static_cast<IntervalExpr*>(right_interval.get())->to_str());
        } else if ((left_interval->type() == AstType::Interval) &&
                   right_interval->type() == AstType::IntervalUnion) {
            return fmt::format(
                "{} union {}",
                static_cast<IntervalExpr*>(left_interval.get())->to_str(),
                static_cast<IntervalUnion*>(right_interval.get())->to_str());
        } else if ((left_interval->type() == AstType::Interval) &&
                   right_interval->type() == AstType::Interval) {
            return fmt::format(
                "{} union {}",
                static_cast<IntervalExpr*>(left_interval.get())->to_str(),
                static_cast<IntervalExpr*>(right_interval.get())->to_str());
        }
    }
};
// later cuz me is epic
struct SetObject : public Expr {
    using inner_t = std::variant<Number>;
    std::set<inner_t> value;
};
struct Matrix : public Expr {};
}  // namespace ami
