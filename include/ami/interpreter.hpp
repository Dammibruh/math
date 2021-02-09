#pragma once
#include <fmt/core.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include "builtins.hpp"
#include "errors.hpp"
#include "parser.hpp"

namespace ami {
using val_t = std::variant<Number, Boolean, std::string>;
namespace scope {
static std::unordered_map<std::string, val_t> userdefined;
static std::unordered_map<std::string, ami::Function> userdefined_functions;
}  // namespace scope
class Interpreter {
    using u_ptr = std::shared_ptr<Expr>;
    std::unordered_map<std::string, val_t> arguments_scope;
    std::size_t call_count = 0;
    std::size_t max_call_count = 1'000;
    std::size_t m_Pos = 0;
    ami::exceptions::ExceptionInterface ei;
    // exceptions
    void m_ThrowErr(const std::string& str, const std::string& msg,
                    std::optional<std::size_t> pos = std::nullopt) {
        ei.name = str;
        ei.err = msg;
        ei.linepos = pos.value_or(m_Pos);
        throw ami::exceptions::BaseException(ei);
    }
    void m_Err(const std::string& msg,
               std::optional<std::size_t> pos = std::nullopt) {
        m_ThrowErr("Error", msg, pos);
    }
    bool m_IsValidOper(const val_t& vr) {
        // to make operations only valid between numbers
        return std::get_if<Number>(&vr) != nullptr;
    }
    val_t m_VisitAdd(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::get<Number>(_lhs).val +
                          std::get<Number>(_rhs).val);
        else
            m_Err("binary operation '+' is only valid for numbers");
    }
    val_t m_VisitSub(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::get<Number>(_lhs).val -
                          std::get<Number>(_rhs).val);
        else
            m_Err("binary operation '-' is only valid for numbers");
    }
    val_t m_VisitDiv(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::get<Number>(_lhs).val /
                          std::get<Number>(_rhs).val);
        else
            m_Err("binary operation '/' is only valid for numbers");
    }
    val_t m_VisitMult(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::get<Number>(_lhs).val *
                          std::get<Number>(_rhs).val);
        else
            m_Err("binary operation '*' is only valid for numbers");
    }
    val_t m_VisitIdent(Identifier* ident) {
        bool is_a_function_arg =
            arguments_scope.find(ident->name) != arguments_scope.end();
        bool is_a_defined_ident =
            scope::userdefined.find(ident->name) != scope::userdefined.end();
        bool is_a_builtin_ident = ami::builtins::constants.find(ident->name) !=
                                  ami::builtins::constants.end();
        if (is_a_function_arg) {
            return arguments_scope.at(ident->name);
        } else if (is_a_builtin_ident) {
            return Number(ami::builtins::constants.at(ident->name));
        } else if ((!scope::userdefined.empty()) && is_a_defined_ident) {
            return scope::userdefined.at(ident->name);
        } else {
            m_Err(
                fmt::format("use of undeclared identifier '{}'", ident->name));
        }
    }
    val_t m_VisitUserDefinedIdentifier(UserDefinedIdentifier* udi) {
        bool is_builtin = ami::builtins::constants.find(udi->name) !=
                          ami::builtins::constants.end();
        if (is_builtin) {
            m_Err(fmt::format("Can't assign to built-in identifier '{}'",
                              udi->name));
        } else {
            auto visited = visit(udi->value);
            scope::userdefined.insert_or_assign(udi->name, visited);
            return fmt::format("defined identifier '{}'", udi->name);
        }
    }
    val_t m_VisitFunction(FunctionCall* fc) {
        // recursion check, haha die segmentation fault
        call_count++;
        if (call_count >= max_call_count) {
            std::size_t temp = call_count;
            call_count = 0;
            m_Err(fmt::format("{} called recursively for {}", fc->name, temp),
                  0);
        }
        decltype(ami::builtins::functions)::iterator get_builtin =
            ami::builtins::functions.find(fc->name);
        bool is_builtin = get_builtin != ami::builtins::functions.end();
        auto get_userdefined = scope::userdefined_functions.find(fc->name);
        bool is_userdefined =
            get_userdefined != scope::userdefined_functions.end();
        // helper variables
        std::vector<std::shared_ptr<ami::Expr>> args = fc->arguments;
        if (is_builtin) {
            std::vector<val_t> parsed_args;
            if (args.size() != get_builtin->second.args_count) {
                m_Err(fmt::format(
                    "mismatched arguments for function call '{}' "
                    ",called with {} argument, expected {} ",
                    fc->name, args.size(), get_builtin->second.args_count));
            }
            // evaluate each passed argument
            std::for_each(args.begin(), args.end(),
                          [&parsed_args, this](const auto& arg) {
                              parsed_args.push_back(visit(arg));
                          });
            return Number(get_builtin->second.callback(parsed_args));
        } else if (is_userdefined) {
            if (args.size() != get_userdefined->second.arguments.size()) {
                m_Err(
                    fmt::format("mismatched arguments for function call '{}' "
                                ",called with {} argument, expected {} ",
                                fc->name, args.size(),
                                get_userdefined->second.arguments.size()));
            }
            std::vector<std::shared_ptr<Expr>> fc_args =
                get_userdefined->second.arguments;
            for (decltype(fc_args)::size_type i = 0; i < fc_args.size(); i++) {
                Identifier* ident =
                    static_cast<Identifier*>(fc_args.at(i).get());
                std::string cname = ident->name;
                arguments_scope.insert_or_assign(cname, visit(args.at(i)));
            }
            std::shared_ptr<Expr> fc_body = get_userdefined->second.body;
            // the function's body is evaluated only when it's called
            return visit(fc_body);

        } else {
            m_Err(fmt::format("use of undeclared function '{}'", fc->name));
        }
    }
    val_t m_VisitFunctionDef(Function* func) {
        std::string name = func->name;
        bool is_builtin = ami::builtins::functions.find(name) !=
                          ami::builtins::functions.end();
        if (is_builtin) {
            m_Err(fmt::format("can't assign to built-in function '{}'", name));
        } else {
            scope::userdefined_functions.insert_or_assign(
                name, Function(func->name, func->body, func->arguments));
            return fmt::format("defined function '{}'", name);
        }
    }
    val_t m_VisitMod(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::fmod(std::get<Number>(_lhs).val,
                                    std::get<Number>(_rhs).val));
        else
            m_Err("binary operation '%%' is only valid for numbers");
    }
    val_t m_VisitPow(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::pow(std::get<Number>(_lhs).val,
                                   std::get<Number>(_rhs).val));
        else
            m_Err("binary operation '^' is only valid for numbers");
    }
    val_t m_VisitNegative(NegativeExpr* ex) {
        val_t val = visit(ex->value);
        if (m_IsValidOper(val))
            return Number(-std::get<Number>(val).val);
        else
            m_Err("unary operator '-' is only valid for numbers");
    }
    bool m_IsBoolOrNum(const val_t& oht) {}
    val_t m_VisitLogicalNot(LogicalExpr*) {}
    val_t m_VisitLogicalAnd(LogicalExpr* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        bool lhs_is_number = lhs_get_number != nullptr;
        bool lhs_is_bool = lhs_get_bool != nullptr;
        bool rhs_is_number = rhs_get_number != nullptr;
        bool rhs_is_bool = rhs_get_bool != nullptr;
        if (lhs_is_number && rhs_is_number) {
            return Boolean(lhs_get_number->val && rhs_get_number->val);
        } else if (lhs_is_bool && rhs_is_bool) {
            return Boolean(lhs_get_bool->val && rhs_get_bool->val);
        } else if (lhs_is_number && rhs_is_bool) {
            return Boolean(lhs_get_number->val && rhs_get_bool->val);
        } else if (lhs_is_bool && rhs_is_number) {
            return Boolean(lhs_get_bool->val && rhs_get_number->val);
        } else {
            m_Err("logical 'and' is only valid for numbers and booleans");
        }
    }
    val_t m_VisitLogicalOr(LogicalExpr* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        bool lhs_is_number = lhs_get_number != nullptr;
        bool lhs_is_bool = lhs_get_bool != nullptr;
        bool rhs_is_number = rhs_get_number != nullptr;
        bool rhs_is_bool = rhs_get_bool != nullptr;
        if (lhs_is_number && rhs_is_number) {
            return Boolean(lhs_get_number->val || rhs_get_number->val);
        } else if (lhs_is_bool && rhs_is_bool) {
            return Boolean(lhs_get_bool->val || rhs_get_bool->val);
        } else if (lhs_is_number && rhs_is_bool) {
            return Boolean(lhs_get_number->val || rhs_get_bool->val);
        } else if (lhs_is_bool && rhs_is_number) {
            return Boolean(lhs_get_bool->val || rhs_get_number->val);
        } else {
            m_Err("logical 'or' is only valid for numbers and booleans");
        }
    }
    val_t m_VisitEquals(Comparaison* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        bool lhs_is_number = lhs_get_number != nullptr;
        bool lhs_is_bool = lhs_get_bool != nullptr;
        bool rhs_is_number = rhs_get_number != nullptr;
        bool rhs_is_bool = rhs_get_bool != nullptr;
        if (lhs_is_number && rhs_is_number) {
            return (Boolean(lhs_get_number->val == rhs_get_number->val));
        } else if (lhs_is_bool && rhs_is_bool) {
            return (Boolean(lhs_get_bool->val == rhs_get_bool->val));
        } else if (lhs_is_number && rhs_is_bool) {
            return (Boolean(lhs_get_number->val == rhs_get_bool->val));
        } else if (lhs_is_bool && rhs_is_number) {
            return (Boolean(lhs_get_bool->val == rhs_get_number->val));
        } else {
            m_Err(
                "comparaison operator '==' is only valid for numbers and "
                "booleans");
        }
    }
    val_t m_VisitGreaterThan(Comparaison* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        bool lhs_is_number = lhs_get_number != nullptr;
        bool lhs_is_bool = lhs_get_bool != nullptr;
        bool rhs_is_number = rhs_get_number != nullptr;
        bool rhs_is_bool = rhs_get_bool != nullptr;
        if (lhs_is_number && rhs_is_number) {
            return (Boolean(lhs_get_number->val > rhs_get_number->val));
        } else if (lhs_is_bool && rhs_is_bool) {
            return (Boolean(lhs_get_bool->val > rhs_get_bool->val));
        } else if (lhs_is_number && rhs_is_bool) {
            return (Boolean(lhs_get_number->val > rhs_get_bool->val));
        } else if (lhs_is_bool && rhs_is_number) {
            return (Boolean(lhs_get_bool->val > rhs_get_number->val));
        } else {
            m_Err(
                "comparaison operator '>' is only valid for numbers and "
                "booleans");
        }
    }
    val_t m_VisitLess(Comparaison* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        bool lhs_is_number = lhs_get_number != nullptr;
        bool lhs_is_bool = lhs_get_bool != nullptr;
        bool rhs_is_number = rhs_get_number != nullptr;
        bool rhs_is_bool = rhs_get_bool != nullptr;
        if (lhs_is_number && rhs_is_number) {
            return (Boolean(lhs_get_number->val < rhs_get_number->val));
        } else if (lhs_is_bool && rhs_is_bool) {
            return (Boolean(lhs_get_bool->val < rhs_get_bool->val));
        } else if (lhs_is_number && rhs_is_bool) {
            return (Boolean(lhs_get_number->val < rhs_get_bool->val));
        } else if (lhs_is_bool && rhs_is_number) {
            return (Boolean(lhs_get_bool->val < rhs_get_number->val));
        } else {
            m_Err(
                "comparaison operator '<' is only valid for numbers and "
                "booleans");
        }
    }
    val_t m_VisitLessOrEqual(Comparaison* comp) {
        auto l = m_VisitLess(comp);
        auto r = m_VisitEquals(comp);
        return Boolean(std::get_if<Boolean>(&l)->val ||
                       std::get_if<Boolean>(&r)->val);
    }
    val_t m_VisitGreaterOrEqual(Comparaison* comp) {
        auto l = m_VisitGreaterThan(comp);
        auto r = m_VisitEquals(comp);
        return Boolean(std::get_if<Boolean>(&l)->val ||
                       std::get_if<Boolean>(&r)->val);
    }
    val_t m_VisitIfExpr(IfExpr* iexpr) {
        auto _cond = visit(iexpr->cond);
        Boolean* get_bool = std::get_if<Boolean>(&_cond);
        Number* get_num = std::get_if<Number>(&_cond);
        std::string* get_str = std::get_if<std::string>(&_cond);
        bool is_bool = get_bool != nullptr;
        bool is_num = get_num != nullptr;
        bool is_str = get_str != nullptr;
        bool is_true = is_bool
                           ? get_bool->val
                           : (is_num ? get_num->val
                                     : (is_str ? !get_str->empty() : false));
        if (is_true) {
            return visit(iexpr->body);
        } else if (iexpr->elsestmt != nullptr) {
            return visit(iexpr->elsestmt);
        } else {
            return Boolean(false);
        }
    }
    val_t m_VisitNumber(Number* num) { return Number(num->val); }
    val_t m_VisitBool(Boolean* _b) { return *_b; }

   public:
    Interpreter(const ami::exceptions::ExceptionInterface& ei) : ei(ei){};
    val_t visit(u_ptr expr) {
        m_Pos++;
        switch (expr->type()) {
            default: {
                m_Err("invalid expression");
            }
            case AstType::BinaryOp: {
                BinaryOpExpr* bopexpr = static_cast<BinaryOpExpr*>(expr.get());
                switch (bopexpr->op) {
                    default:
                        m_Err("invalid operator");
                        break;
                    case Op::Plus:
                        return m_VisitAdd(bopexpr);
                    case Op::Minus:
                        return m_VisitSub(bopexpr);
                    case Op::Div:
                        return m_VisitDiv(bopexpr);
                    case Op::Mult:
                        return m_VisitMult(bopexpr);
                    case Op::Pow:
                        return m_VisitPow(bopexpr);
                    case Op::Mod:
                        return m_VisitMod(bopexpr);
                }
            }
            case AstType::LogicalExpr: {
                LogicalExpr* lexpr = static_cast<LogicalExpr*>(expr.get());
                switch (lexpr->op) {
                    default:
                        m_Err("invalid logical operator");
                    case Op::LogicalAnd:
                        return m_VisitLogicalAnd(lexpr);
                    case Op::LogicalOr:
                        return m_VisitLogicalOr(lexpr);
                }
            }
            case AstType::Comparaison: {
                Comparaison* comp = static_cast<Comparaison*>(expr.get());
                switch (comp->op) {
                    default:
                        m_Err("invalid comparaison operator");
                    case Op::Greater:
                        return m_VisitGreaterThan(comp);
                    case Op::GreaterOrEqual:
                        return m_VisitGreaterOrEqual(comp);
                    case Op::Less:
                        return m_VisitLess(comp);
                    case Op::LessOrEqual:
                        return m_VisitLessOrEqual(comp);
                    case Op::Equals:
                        return m_VisitEquals(comp);
                }
            }
            case AstType::Number: {
                return m_VisitNumber(static_cast<Number*>(expr.get()));
            }
            case AstType::Identifier: {
                return m_VisitIdent(static_cast<Identifier*>(expr.get()));
            }
            case AstType::UserDefinedIdentifier: {
                return m_VisitUserDefinedIdentifier(
                    static_cast<UserDefinedIdentifier*>(expr.get()));
            }
            case AstType::FunctionCall: {
                return m_VisitFunction(static_cast<FunctionCall*>(expr.get()));
            }
            case AstType::Function: {
                return m_VisitFunctionDef(static_cast<Function*>(expr.get()));
            }
            case AstType::NegativeExpr: {
                return m_VisitNegative(static_cast<NegativeExpr*>(expr.get()));
            }
            case AstType::Boolean: {
                return m_VisitBool(static_cast<Boolean*>(expr.get()));
            }
            case AstType::IfExpr: {
                return m_VisitIfExpr(static_cast<IfExpr*>(expr.get()));
            }
        }
    }
    val_t visitvec(const std::vector<u_ptr>& exprs) {
        val_t out = visit(exprs.at(0));
        for (auto& elm : exprs) out = visit(elm);
        return out;
    }
    ~Interpreter() = default;
};
}  // namespace ami
