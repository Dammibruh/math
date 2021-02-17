#pragma once
#include <fmt/core.h>

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
    MinusAssign,
    Plus,
    PlusAssign,
    Div,
    DivAssign,
    Mult,
    MultAssign,
    Pow,
    PowAssign,
    Mod,
    ModAssign,
    LogicalAnd,
    LogicalOr,
    Equals,
    NotEquals,
    Greater,
    Less,
    GreaterOrEqual,
    LessOrEqual
};
enum class AstType {
    Number,
    BinaryOp,
    OpAndAssign,
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
    Factorial,
    ReturnExpr,
    SetObject,
    Matrix,
    Vector,
    Tensor,
    Interval,
    SliceExpr,
    InExpr,
    UnionExpr,
    IntersectionExpr,
    Comparison,
    LogicalExpr
};
static std::map<Op, std::string_view> ops_str{
    {Op::Minus, "-"},        {Op::Plus, "+"},
    {Op::Div, "/"},          {Op::Mult, "*"},
    {Op::Pow, "^"},          {Op::Mod, "%"},
    {Op::MinusAssign, "-="}, {Op::PlusAssign, "+="},
    {Op::DivAssign, "/="},   {Op::MultAssign, "*="},
    {Op::PowAssign, "^="},   {Op::ModAssign, "%="},
    {Op::LogicalAnd, "and"}, {Op::LogicalOr, "or"},
    {Op::Greater, ">"},      {Op::GreaterOrEqual, ">="},
    {Op::Less, "<"},         {Op::LessOrEqual, "<="},
    {Op::Equals, "=="},      {Op::NotEquals, "!="}};
struct Expr {
    virtual std::string str() = 0;
    virtual AstType type() const = 0;
    virtual std::string to_str() = 0;
    virtual ~Expr() = default;
};
struct Number : public Expr {
    long double val;
    explicit Number(long double x) : val(x) {}
    std::string str() override {
        return fmt::format("<Number value=<{}>>", val);
    }
    AstType type() const override { return AstType::Number; }
    bool operator<(const Number& n) const { return val < n.val; }
    bool operator>(const Number& n) const { return val > n.val; }
    bool operator==(const Number& n) const { return val == n.val; }
    long double operator*(const Number& n) const { return (val * n.val); }
    std::string to_str() override {
        std::stringstream ss;
        ss << std::setprecision(15);
        ss << val;
        return ss.str();
    }
};
struct FactorialExpr : public Expr {
    std::shared_ptr<Expr> value;
    explicit FactorialExpr(const std::shared_ptr<Expr>& v) : value(v) {}
    std::string str() override {
        return fmt::format("<FactorialExpr value=<{}>>", value->str());
    }
    AstType type() const override { return AstType::Factorial; }
    std::string to_str() override { return fmt::format("{}!", value->str()); }
};
struct Boolean : public Expr {
    bool val;
    std::string raw_value;
    explicit Boolean(std::string_view _val) : raw_value(_val) {
        if (_val == "true")
            val = true;
        else if (_val == "false")
            val = false;
    }
    explicit Boolean(bool _val) : val(_val) {
        if (_val)
            raw_value = "true";
        else if (!_val)
            raw_value = "false";
    }
    AstType type() const override { return AstType::Boolean; }
    std::string str() override {
        return fmt::format("<Boolean value={}>", raw_value);
    }
    std::string to_str() override { return raw_value; }
};
struct Comparison : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    Comparison(Op o, const std::shared_ptr<Expr>& l,
               const std::shared_ptr<Expr>& r)
        : lhs(l), rhs(r), op(o) {}
    AstType type() const override { return AstType::Comparison; }
    std::string str() override {
        return fmt::format(
            "<Comparison left=<{left}> op=<{op}>, right=<{right}>>",
            fmt::arg("left", lhs->str()), fmt::arg("op", ops_str.at(op)),
            fmt::arg("right", rhs->str()));
    }
    std::string to_str() override {
        return fmt::format("{} {} {}", lhs->to_str(), ops_str.at(op),
                           rhs->to_str());
    }
};
struct LogicalExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    LogicalExpr(Op o, const std::shared_ptr<Expr>& l,
                const std::shared_ptr<Expr>& r)
        : lhs(l), rhs(r), op(o) {}
    AstType type() const override { return AstType::LogicalExpr; }
    std::string str() override {
        return fmt::format("<LogicalExpr lhs=<{l}>, op=<{op}>, rhs=<{r}>>",
                           fmt::arg("l", lhs->str()), fmt::arg("r", rhs->str()),
                           fmt::arg("op", ops_str.at(op)));
    }
    std::string to_str() override {
        return fmt::format("{} {} {}", lhs->to_str(), ops_str.at(op),
                           rhs->to_str());
    }
};
struct NegativeExpr : public Expr {
    std::shared_ptr<Expr> value;
    explicit NegativeExpr(const std::shared_ptr<Expr>& val) : value(val) {}
    AstType type() const override { return AstType::NegativeExpr; }
    std::string str() override {
        return fmt::format("<NegativeExpr value=<{}>>", value->str());
    }
    std::string to_str() override {
        return fmt::format("-{}", value->to_str());
    }
};
struct NotExpr : public Expr {
    std::shared_ptr<Expr> value;
    explicit NotExpr(const std::shared_ptr<Expr>& val) : value(val) {}
    std::string str() override {
        return fmt::format("<NotExpr value=<{}>>", value->str());
    }
    std::string to_str() override {
        return fmt::format("not {}", value->to_str());
    }
    AstType type() const override { return AstType::NotExpr; }
};
struct NullExpr : public Expr {
    std::string value = "null";
    NullExpr() {}
    std::string str() override { return "<NullExpr>"; }
    AstType type() const override { return AstType::NullExpr; }
    std::string to_str() override { return value; }
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
    std::string to_str() override {
        return fmt::format("{} {} {}", lhs->to_str(), ops_str.at(op),
                           rhs->to_str());
    }
};
struct OpAndAssignExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    Op op;
    OpAndAssignExpr(Op c, const std::shared_ptr<Expr>& _lhs,
                    const std::shared_ptr<Expr>& _rhs)
        : lhs(_lhs), rhs(_rhs), op(c) {}
    std::string str() override {
        return fmt::format("<BinaryOpExpr left=<{}>, right=<{}>>", lhs->str(),
                           rhs->str());
    }
    AstType type() const override { return AstType::OpAndAssign; }
    std::string to_str() override {
        return fmt::format("{} {} {}", lhs->to_str(), ops_str.at(op),
                           rhs->to_str());
    }
};
struct Identifier : public Expr {
    std::string name;
    explicit Identifier(const std::string& name) : name(name) {}
    std::string str() override {
        return fmt::format("<Identifier name=<{}>>", name);
    }
    AstType type() const override { return AstType::Identifier; }
    std::string to_str() override { return name; }
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
    std::string to_str() override {
        return fmt::format("x = {}", value->to_str());
    }
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
                ss << arg->str();
            }
            ss << ">>";
        }
        return ss.str();
    }
    std::string to_str() override { return "function call"; }
};
struct Function : public Expr {
    std::string name;
    std::size_t call_count;
    std::shared_ptr<Expr> body, ReturnStmt;
    std::vector<std::shared_ptr<Expr>> arguments;
    Function(std::string_view name, const std::shared_ptr<Expr>& body,
             const std::vector<std::shared_ptr<Expr>>& args)
        : name(name), call_count(0), body(body), arguments(args) {}
    AstType type() const override { return AstType::Function; }
    std::string str() override {
        std::string str;
        str += ("<Function name=<" + name + ">, args=<");
        if (arguments.size() > 0) {
            for (auto& ar : arguments) str += ar->str() + ", ";
        } else {
            str += "null";
        }
        str += fmt::format(">, body=<{}>>", body->str());
        return str;
    }
    std::string to_str() override { return fmt::format("function '{}'", name); }
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
    std::string to_str() override { return "if statement"; }
};
struct IntervalHandler {
    std::shared_ptr<Expr> value;
    bool strict;
    explicit IntervalHandler(const std::shared_ptr<Expr>& v, bool s = false)
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
    std::string to_str() override {
        char lft = min.strict ? ']' : '[';
        char rgt = max.strict ? '[' : ']';
        return fmt::format(
            "{}{}; {}{}", lft, static_cast<Number*>(min.value.get())->to_str(),
            static_cast<Number*>(max.value.get())->to_str(), rgt);
    }
};
struct InExpr : public Expr {
    std::shared_ptr<Expr> number;
    std::shared_ptr<Expr> inter;  // no worries the cool guy interpreter will
                                  // handle this parser is just for syntax
    InExpr(const std::shared_ptr<Expr>& h, const std::shared_ptr<Expr>& inter)
        : number(h), inter(inter) {}
    InExpr(const InExpr& oth) = default;
    std::string str() override {
        return fmt::format("<InExpr number=<{}>, interval=<{}>>", number->str(),
                           inter->str());
    }
    AstType type() const override { return AstType::InExpr; }
    std::string to_str() override {
        return fmt::format("{} in {}", number->to_str(), inter->to_str());
    }
};
struct UnionExpr : public Expr {
    std::shared_ptr<Expr> left_interval, right_interval;
    UnionExpr(const std::shared_ptr<Expr>& h,
              const std::shared_ptr<Expr>& inter)
        : left_interval(h), right_interval(inter) {}
    UnionExpr(const UnionExpr& oth) = default;
    AstType type() const override { return AstType::UnionExpr; }
    std::string str() override {
        return fmt::format("<UnionExpr left=<{}>, right=<{}>>",
                           left_interval->str(), right_interval->str());
    }
    std::string to_str() override {
        return fmt::format("{} union {}", left_interval->to_str(),
                           right_interval->to_str());
    }
};
struct InterSectionExpr : public Expr {
    std::shared_ptr<Expr> lhs, rhs;
    InterSectionExpr(const std::shared_ptr<Expr>& h,
                     const std::shared_ptr<Expr>& inter)
        : lhs(h), rhs(inter) {}
    InterSectionExpr(const InterSectionExpr& oth) = default;
    AstType type() const override { return AstType::IntersectionExpr; }
    std::string str() override {
        return fmt::format("<IntersectionExpr left=<{}>, right=<{}>>",
                           lhs->str(), rhs->str());
    }
    std::string to_str() override {
        return fmt::format("{} intersection {}", lhs->to_str(), rhs->to_str());
    }
};
namespace details {
static std::string vecToString(const std::vector<std::shared_ptr<Expr>>& vec,
                               std::string_view start = "{",
                               std::string_view end = "}") {
    std::string out{start};
    if (!vec.empty()) {
        for (std::size_t x = 0; x < vec.size(); ++x) {
            if (x == (vec.size() - 1))
                out += vec.at(x)->to_str();
            else
                out += vec.at(x)->to_str() + ", ";
        }
    }
    out += end;
    return out;
}
}  // namespace details
struct SetObject : public Expr {
    // checking and stuff is handled by the interpreter cuz too lazy
    // to write a non sense virtual overload spaceship operator and
    // override it why tf would I overload it for
    // intervals/comparison/logical expressions 😉
    std::vector<std::shared_ptr<Expr>> value;
    explicit SetObject(const std::vector<std::shared_ptr<Expr>>& _v)
        : value(_v) {}
    AstType type() const override { return AstType::SetObject; }
    std::string str() override {
        std::string _str{"<SetObject value={"};
        if (value.empty()) {
            _str += "null";
        } else {
            for (auto& e : value) {
                _str += e->str() + ", ";
            }
        }
        _str += "}>";
        return _str;
    }
    std::string to_str() override { return details::vecToString(value); }
    bool operator<(const SetObject& oth) const { return value < oth.value; }
    bool operator>(const SetObject& oth) const { return value > oth.value; }
    bool operator==(const SetObject& oth) const { return value == oth.value; }
};
struct SliceExpr : public Expr {
    std::shared_ptr<Expr> target, index;
    SliceExpr(const std::shared_ptr<Expr>& sr, std::shared_ptr<Expr> i)
        : target(sr), index(i) {}
    AstType type() const override { return AstType::SliceExpr; }
    std::string to_str() override { return target->to_str(); }
    std::string str() override {
        return fmt::format("<SliceExpr target<{}> index=<{}>>", target->str(),
                           index->str());
    }
};
struct SetOpExpr : public Expr {};
struct Vector : public Expr {
    std::vector<std::shared_ptr<Expr>> value;
    explicit Vector(const std::vector<std::shared_ptr<Expr>>& v) : value(v) {}
    AstType type() const override { return AstType::Vector; }
    std::string str() override {
        std::string _str{"<Vector value={"};
        if (value.empty()) {
            _str += "null";
        } else {
            for (auto& e : value) {
                _str += e->str() + ", ";
            }
        }
        _str += "}>";
        return _str;
    }
    std::string to_str() override {
        return details::vecToString(value, "[", "]");
    }
    bool operator<(const Vector& oth) const { return value < oth.value; }
    bool operator>(const Vector& oth) const { return value > oth.value; }
    bool operator==(const Vector& oth) const { return value == oth.value; }
};
struct Matrix : public Expr {
    std::vector<std::shared_ptr<Expr>>
        value;  // cool interpreter will do the job
    explicit Matrix(const std::vector<std::shared_ptr<Expr>>& v) : value(v) {}
    AstType type() const override { return AstType::Matrix; }
    std::string str() override {
        std::string _str{"<Matrix value={"};
        if (value.size()) {
            _str += "null";
        } else {
            for (auto& e : value) {
                _str += e->str() + ", ";
            }
        }
        _str += "}>";
        return _str;
    }
    std::string to_str() override {
        return details::vecToString(value, "[", "]");
    }
};
}  // namespace ami
