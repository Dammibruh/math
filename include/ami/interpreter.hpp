#pragma once
#include <fmt/core.h>

#include <algorithm>
#include <cmath>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "builtins.hpp"
#include "errors.hpp"
#include "parser.hpp"
#include "types.hpp"

namespace ami {
namespace scope {
static ami::iscope_t userdefined;
static ami::fscope_t userdefined_functions;
}  // namespace scope
class Interpreter {
    using nested_scope_t = std::vector<iscope_t>;
    std::size_t max_call_count = 3'000;
    std::size_t m_Pos = 0;
    ami::exceptions::ExceptionInterface ei;
    nested_scope_t arguments_scope;
    // exceptions
    void m_ThrowErr(const std::string& str, const std::string& msg,
                    std::optional<std::size_t> pos = std::nullopt) {
        ei.name = str;
        ei.err = msg;
        ei.linepos = pos.value_or(m_Pos);
        throw ami::exceptions::BaseException(ei);
    }
    void m_CheckOrErr(bool t_y, const std::string& msg) {
        if (!t_y) m_Err(msg);
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
            m_Err("binary operation '+' is not valid in this context");
    }
    val_t m_VisitSub(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs)) {
            return Number(std::get<Number>(_lhs).val -
                          std::get<Number>(_rhs).val);
        } else if (auto [lhs, rhs] = std::tuple{std::get_if<SetObject>(&_lhs),
                                                std::get_if<SetObject>(&_rhs)};
                   (lhs != nullptr) && (rhs != nullptr)) {
            std::vector<ptr_t> val;
            std::vector<Number> f_temp;
            for (auto& e : rhs->value) {
                val_t t_v_num = visit(e);
                Number* t_num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(t_num != nullptr, "invalid type");
                f_temp.push_back(*t_num);
            }
            for (auto& e : lhs->value) {
                val_t t_v_num = visit(e);
                Number* t_num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(t_num != nullptr, "invalid type");
                if (std::find(f_temp.begin(), f_temp.end(), *t_num) ==
                    f_temp.end()) {
                    val.push_back(e);
                }
            }
            return visit(std::make_shared<SetObject>(val));
        } else {
            m_Err("binary operation '-' is not valid in this context");
        }
    }
    val_t m_VisitDiv(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::get<Number>(_lhs).val /
                          std::get<Number>(_rhs).val);
        else
            m_Err("binary operation '/' is not valid in this context");
    }
    val_t m_VisitMult(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs)) {
            return Number(std::get<Number>(_lhs).val *
                          std::get<Number>(_rhs).val);
        } else if (auto [lhs, rhs] = std::tuple{std::get_if<Number>(&_lhs),
                                                std::get_if<Vector>(&_rhs)};
                   (lhs != nullptr && rhs != nullptr)) {
            std::vector<ptr_t> out;
            for (auto& elm : rhs->value) {
                val_t t_v_num = visit(elm);
                Number* num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(num != nullptr, "invalid vector type");
                out.push_back(std::make_shared<Number>(num->val * lhs->val));
            }
            return visit(std::make_shared<Vector>(out));
        } else if (auto [lhs, rhs] = std::tuple{std::get_if<Vector>(&_lhs),
                                                std::get_if<Number>(&_rhs)};
                   (lhs != nullptr && rhs != nullptr)) {
            std::vector<ptr_t> out;
            for (auto& elm : lhs->value) {
                val_t t_v_num = visit(elm);
                Number* num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(num != nullptr, "invalid vector type");
                out.push_back(std::make_shared<Number>(num->val * rhs->val));
            }
            return visit(std::make_shared<Vector>(out));
        } else if (auto [lhs, rhs] = std::tuple{std::get_if<Vector>(&_lhs),
                                                std::get_if<Vector>(&_rhs)};
                   (lhs != nullptr && rhs != nullptr)) {
            m_CheckOrErr(lhs->value.size() == rhs->value.size(),
                         "vectors must have the same size");
            std::vector<Number> lhs_val;
            long double fnl = 0;  // dot product
            for (auto& elm : lhs->value) {
                val_t t_v_num = visit(elm);
                Number* num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(num != nullptr, "invalid vector type");
                lhs_val.push_back(*num);
            }
            for (std::size_t i = 0; i < lhs_val.size(); i++) {
                val_t t_v_num = visit(rhs->value.at(i));
                Number* num = std::get_if<Number>(&t_v_num);
                m_CheckOrErr(num != nullptr, "invalid vector type");
                fnl += (num->val * lhs_val.at(i).val);
            }
            return visit(std::make_shared<Number>(fnl));
        } else {
            m_Err("binary operation '*' is not valid in this context");
        }
    }
    val_t m_VisitIdent(Identifier* ident) {
        bool is_a_function = !arguments_scope.empty();
        auto get_arg_ident = [this, &ident] {
            for (auto it = this->arguments_scope.rbegin();
                 it != this->arguments_scope.rend(); ++it) {
                for (auto ti = it->begin(); ti != it->end(); ++ti) {
                    if (ti->first == ident->name) return ti;
                }
            }
            return this->arguments_scope.back().end();
        }();
        bool is_a_function_arg = get_arg_ident != arguments_scope.back().end();
        bool is_a_defined_ident =
            scope::userdefined.find(ident->name) != scope::userdefined.end();
        bool is_a_builtin_ident = ami::builtins::constants.find(ident->name) !=
                                  ami::builtins::constants.end();
        if (is_a_function_arg) {
            return get_arg_ident->second;
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
            return get_builtin->second.callback(parsed_args);
        } else if (is_userdefined) {
            if (args.size() != get_userdefined->second.arguments.size()) {
                m_Err(
                    fmt::format("mismatched arguments for function call '{}' "
                                ",called with {} argument, expected {} ",
                                fc->name, args.size(),
                                get_userdefined->second.arguments.size()));
            }
            if (std::size_t count = get_userdefined->second.call_count;
                count >= max_call_count) {
                get_userdefined->second.call_count = 0;
                m_Err(fmt::format(
                          "function '{}' has been called recursively for {}",
                          fc->name, count),
                      0);
            }
            std::vector<std::shared_ptr<Expr>> fc_args =
                get_userdefined->second.arguments;
            iscope_t tempscope;
            for (decltype(fc_args)::size_type i = 0; i < fc_args.size(); i++) {
                Identifier* ident =
                    static_cast<Identifier*>(fc_args.at(i).get());
                tempscope.insert_or_assign(ident->name, visit(args.at(i)));
            }
            std::shared_ptr<Expr> fc_body = get_userdefined->second.body;
            get_userdefined->second.call_count++;
            arguments_scope.push_back(tempscope);
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
            m_Err("binary operation '%%' is not valid in this context");
    }
    val_t m_VisitPow(BinaryOpExpr* boe) {
        val_t _lhs = visit(boe->lhs);
        val_t _rhs = visit(boe->rhs);
        if (m_IsValidOper(_lhs) && m_IsValidOper(_rhs))
            return Number(std::pow(std::get<Number>(_lhs).val,
                                   std::get<Number>(_rhs).val));
        else
            m_Err("binary operation '^' is not valid in this context");
    }
    val_t m_VisitNegative(NegativeExpr* ex) {
        val_t val = visit(ex->value);
        if (m_IsValidOper(val))
            return Number(-std::get<Number>(val).val);
        else
            m_Err("binary operation '-' is not valid in this context");
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
            m_Err("logical 'and' is not valid in this context");
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
            m_Err("logical 'or' is not valid in this context");
        }
    }
    val_t m_VisitEquals(Comparison* lexpr) {
        val_t _lhs = visit(lexpr->lhs);
        val_t _rhs = visit(lexpr->rhs);
        Number* lhs_get_number = std::get_if<Number>(&_lhs);
        Boolean* lhs_get_bool = std::get_if<Boolean>(&_lhs);
        Number* rhs_get_number = std::get_if<Number>(&_rhs);
        Boolean* rhs_get_bool = std::get_if<Boolean>(&_rhs);
        SetObject* lhs_get_set = std::get_if<SetObject>(&_lhs);
        SetObject* rhs_get_set = std::get_if<SetObject>(&_rhs);
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
        } else if ((lhs_get_set != nullptr) && (rhs_get_set != nullptr)) {
            return m_VisitEqualsSet(lhs_get_set, rhs_get_set);
        } else {
            m_Err("comparison operator '==' is not valid in this context");
        }
    }
    val_t m_VisitNotEquals(Comparison* lexpr) {
        auto v_bool = m_VisitEquals(lexpr);
        return Boolean(!std::get_if<Boolean>(&v_bool)->val);
    }
    val_t m_VisitGreaterThan(Comparison* lexpr) {
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
            m_Err("comparison operator '>' is not valid in this context");
        }
    }
    val_t m_VisitLess(Comparison* lexpr) {
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
            m_Err("comparison operator '<' is not valid in this context");
        }
    }
    val_t m_VisitLessOrEqual(Comparison* comp) {
        auto l = m_VisitLess(comp);
        auto r = m_VisitEquals(comp);
        return Boolean(std::get_if<Boolean>(&l)->val ||
                       std::get_if<Boolean>(&r)->val);
    }
    val_t m_VisitGreaterOrEqual(Comparison* comp) {
        auto l = m_VisitGreaterThan(comp);
        auto r = m_VisitEquals(comp);
        return Boolean(std::get_if<Boolean>(&l)->val ||
                       std::get_if<Boolean>(&r)->val);
    }
    val_t m_VisitOpAndAssignExpr(OpAndAssignExpr* oexpr) {
        m_CheckOrErr(oexpr->lhs->type() == AstType::Identifier,
                     "assign oprators are only valid for identifiers");
        auto t_f_temp = visit(oexpr->rhs);
        Number* t_new_num = std::get_if<Number>(&t_f_temp);
        m_CheckOrErr(t_new_num != nullptr,
                     "only numbers are valid as a right operand");
        Identifier* ident = static_cast<Identifier*>(oexpr->lhs.get());
        auto t_v_value = m_VisitIdent(ident);
        Number* t_num = std::get_if<Number>(&t_v_value);
        m_CheckOrErr(t_num != nullptr,
                     "binary operators are only valid for numbers");
        bool is_ud =
            scope::userdefined.find(ident->name) != scope::userdefined.end();
        m_CheckOrErr(
            is_ud, fmt::format("'{}' isn't a defined identifier", ident->name));
        auto out = visit(std::make_shared<BinaryOpExpr>(
            oexpr->op, std::make_shared<Number>(t_num->val),
            std::make_shared<Number>(t_new_num->val)));
        scope::userdefined.insert_or_assign(ident->name, out);
        return NullExpr{};
    }
    val_t m_VisitIfExpr(IfExpr* iexpr) {
        auto _cond = visit(iexpr->cond);
        Boolean* get_bool = std::get_if<Boolean>(&_cond);
        Number* get_num = std::get_if<Number>(&_cond);
        NullExpr* get_null = std::get_if<NullExpr>(&_cond);
        std::string* get_str = std::get_if<std::string>(&_cond);
        bool is_bool = get_bool != nullptr;
        bool is_num = get_num != nullptr;
        bool is_str = get_str != nullptr;
        bool is_null = get_null != nullptr;
        bool is_true = is_bool
                           ? get_bool->val
                           : (is_num ? get_num->val
                                     : (is_str ? !get_str->empty() : false));
        if (is_true && !(is_null)) {
            return visit(iexpr->body);
        } else if (iexpr->elsestmt != nullptr) {
            return visit(iexpr->elsestmt);
        } else {
            return NullExpr{};
        }
    }
    val_t m_VisitNumber(Number* num) { return Number(num->val); }
    val_t m_VisitNull() { return NullExpr{}; }
    val_t m_VisitBool(Boolean* _b) { return *_b; }
    val_t m_VisitNot(NotExpr* _b) {
        auto value = visit(_b->value);
        if (auto _number = std::get_if<Number>(&value)) {
            return Boolean(!_number->val);
        } else if (auto _bool = std::get_if<Boolean>(&value)) {
            return Boolean(!_bool->val);
        } else if (auto _null = std::get_if<NullExpr>(&value)) {
            return Boolean(true);
        } else {
            m_Err("operator 'not' is only valid for numbers and booleans");
        }
    }
    val_t m_VisitInterval(IntervalExpr* iexpr) {
        val_t s_min = visit(iexpr->min.value), s_max = visit(iexpr->max.value);
        Number *get_n = std::get_if<Number>(&s_min),
               *get_nn = std::get_if<Number>(&s_max);
        if ((get_n == nullptr) || (get_nn == nullptr)) {
            m_Err("intervals can only hold numbers");
        } else if ((std::isinf(get_n->val) && !iexpr->min.strict) ||
                   (std::isinf(get_nn->val) && !iexpr->max.strict) ||
                   (get_n->val > get_nn->val)) {
            m_Err("invalid interval range");
        } else {
            return IntervalExpr(
                IntervalHandler(std::make_shared<Number>(get_n->val),
                                iexpr->min.strict),
                IntervalHandler(std::make_shared<Number>(get_nn->val),
                                iexpr->max.strict));
        }
    }
    bool m_IsInInterval(long double t_num, long double t_min, long double t_max,
                        bool strict_min, bool strict_max) {
        return ((strict_min ? (t_min < t_num) : (t_min <= t_num)) &&
                (strict_max ? (t_max > t_num) : (t_max >= t_num)));
    }
    val_t m_VisitInExpr(InExpr* iexpr) {
        val_t num = visit(iexpr->number), inter = visit(iexpr->inter);
        Number* get_num = std::get_if<Number>(&num);
        SetObject* get_setf = std::get_if<SetObject>(&num);
        IntervalExpr* get_inter = std::get_if<IntervalExpr>(&inter);
        UnionExpr* get_union = std::get_if<UnionExpr>(&inter);
        InterSectionExpr* get_intersection =
            std::get_if<InterSectionExpr>(&inter);
        SetObject* get_set = std::get_if<SetObject>(&inter);
        if (get_union != nullptr && get_num != nullptr) {
            return m_VisitInExprUnionHelper(get_num->val, get_union);
        } else if (get_intersection != nullptr && get_num != nullptr) {
            return m_VisitInExprIntersectionHelper(get_num->val,
                                                   get_intersection);
        } else if (get_num != nullptr && get_set != nullptr) {
            auto search_result = std::find_if(
                get_set->value.begin(), get_set->value.end(),
                [this, get_num](auto& e) {
                    auto visited = visit(e);
                    if (Number* t_number = std::get_if<Number>(&visited)) {
                        return (t_number->val == get_num->val);
                    } else {
                        m_Err("invalid type");
                    }
                });
            return Boolean(search_result != get_set->value.end());
        } else if (get_num != nullptr && get_inter != nullptr) {
            val_t s_min = visit(get_inter->min.value);
            val_t s_max = visit(get_inter->max.value);
            Number *get_min = std::get_if<Number>(&s_min),
                   *get_max = std::get_if<Number>(&s_max);
            m_CheckOrErr((get_min != nullptr) && (get_max != nullptr),
                         "operator 'in' is only valid between numbers and "
                         "intervals");
            return Boolean(m_IsInInterval(get_num->val, get_min->val,
                                          get_max->val, get_inter->min.strict,
                                          get_inter->max.strict));
        } else if (get_setf != nullptr && get_set != nullptr) {
            auto search_result = std::find_if(
                get_set->value.begin(), get_set->value.end(),
                [this, get_setf](auto& e) {
                    auto visited = visit(e);
                    if (SetObject* t_setobj =
                            std::get_if<SetObject>(&visited)) {
                        return (t_setobj->value == get_setf->value);
                    } else {
                        m_Err("invalid type");
                    }
                });
            return Boolean(search_result != get_set->value.end());
        } else {
            m_Err("invalid use of keyword 'in'");
        }
    }
    val_t m_VisitInExprUnionHelper(long double x, UnionExpr* iun) {
        m_CheckOrErr((iun != nullptr), "invalid union");
        val_t t_right_inter = visit(iun->right_interval);
        val_t t_left_inter = visit(iun->left_interval);
        IntervalExpr* right_inter = std::get_if<IntervalExpr>(&t_right_inter);
        IntervalExpr* left_inter = std::get_if<IntervalExpr>(&t_left_inter);
        UnionExpr* right_union = std::get_if<UnionExpr>(&t_right_inter);
        UnionExpr* left_union = std::get_if<UnionExpr>(&t_left_inter);
        if (left_union != nullptr) {
            val_t v_is_in = m_VisitInExprUnionHelper(x, left_union);
            val_t v_n_max = visit(right_inter->max.value);
            val_t v_n_min = visit(right_inter->min.value);
            Number* n_max = std::get_if<Number>(&v_n_max);
            Number* n_min = std::get_if<Number>(&v_n_min);
            Boolean* is_in = std::get_if<Boolean>(&v_is_in);
            return Boolean(is_in->val ||
                           (m_IsInInterval(x, n_min->val, n_max->val,
                                           right_inter->min.strict,
                                           right_inter->max.strict)));
        } else if (right_union != nullptr) {
            val_t v_is_in = m_VisitInExprUnionHelper(x, right_union);
            val_t v_n_max = visit(left_inter->max.value);
            val_t v_n_min = visit(left_inter->min.value);
            Number* n_max = std::get_if<Number>(&v_n_max);
            Number* n_min = std::get_if<Number>(&v_n_min);
            Boolean* is_in = std::get_if<Boolean>(&v_is_in);
            return Boolean(is_in->val ||
                           (m_IsInInterval(x, n_min->val, n_max->val,
                                           left_inter->min.strict,
                                           left_inter->max.strict)));
        } else {
            val_t v_l_max = visit(left_inter->max.value);
            val_t v_l_min = visit(left_inter->min.value);
            val_t v_r_max = visit(right_inter->max.value);
            val_t v_r_min = visit(right_inter->min.value);
            Number* l_max = std::get_if<Number>(&v_l_max);
            Number* l_min = std::get_if<Number>(&v_l_min);
            Number* r_max = std::get_if<Number>(&v_r_max);
            Number* r_min = std::get_if<Number>(&v_r_min);
            return Boolean(m_IsInInterval(x, l_min->val, l_max->val,
                                          left_inter->min.strict,
                                          left_inter->max.strict) ||
                           m_IsInInterval(x, r_min->val, r_max->val,
                                          right_inter->min.strict,
                                          right_inter->max.strict));
        }
    }
    val_t m_VisitInExprIntersectionHelper(long double x,
                                          InterSectionExpr* iun) {
        m_CheckOrErr((iun != nullptr), "invalid intersection");
        val_t t_right_inter = visit(iun->rhs);
        val_t t_left_inter = visit(iun->lhs);
        IntervalExpr* right_inter = std::get_if<IntervalExpr>(&t_right_inter);
        IntervalExpr* left_inter = std::get_if<IntervalExpr>(&t_left_inter);
        InterSectionExpr* right_intersection =
            std::get_if<InterSectionExpr>(&t_right_inter);
        InterSectionExpr* left_intersection =
            std::get_if<InterSectionExpr>(&t_left_inter);
        if (left_intersection != nullptr) {
            val_t v_is_in =
                m_VisitInExprIntersectionHelper(x, left_intersection);
            val_t v_n_max = visit(right_inter->max.value);
            val_t v_n_min = visit(right_inter->min.value);
            Number* n_max = std::get_if<Number>(&v_n_max);
            Number* n_min = std::get_if<Number>(&v_n_min);
            Boolean* is_in = std::get_if<Boolean>(&v_is_in);
            return Boolean(is_in->val &&
                           (m_IsInInterval(x, n_min->val, n_max->val,
                                           right_inter->min.strict,
                                           right_inter->max.strict)));
        } else if (right_intersection != nullptr) {
            val_t v_is_in =
                m_VisitInExprIntersectionHelper(x, right_intersection);
            val_t v_n_max = visit(left_inter->max.value);
            val_t v_n_min = visit(left_inter->min.value);
            Number* n_max = std::get_if<Number>(&v_n_max);
            Number* n_min = std::get_if<Number>(&v_n_min);
            Boolean* is_in = std::get_if<Boolean>(&v_is_in);
            return Boolean(is_in->val &&
                           (m_IsInInterval(x, n_min->val, n_max->val,
                                           left_inter->min.strict,
                                           left_inter->max.strict)));
        } else {
            val_t v_l_max = visit(left_inter->max.value);
            val_t v_l_min = visit(left_inter->min.value);
            val_t v_r_max = visit(right_inter->max.value);
            val_t v_r_min = visit(right_inter->min.value);
            Number* l_max = std::get_if<Number>(&v_l_max);
            Number* l_min = std::get_if<Number>(&v_l_min);
            Number* r_max = std::get_if<Number>(&v_r_max);
            Number* r_min = std::get_if<Number>(&v_r_min);
            return Boolean(m_IsInInterval(x, l_min->val, l_max->val,
                                          left_inter->min.strict,
                                          left_inter->max.strict) &&
                           m_IsInInterval(x, r_min->val, r_max->val,
                                          right_inter->min.strict,
                                          right_inter->max.strict));
        }
    }
    val_t m_VisitUnionExpr(UnionExpr* iun) {
        val_t _l = visit(iun->left_interval), _r = visit(iun->right_interval);
        IntervalExpr *left_inter = std::get_if<IntervalExpr>(&_l),
                     *right_inter = std::get_if<IntervalExpr>(&_r);
        SetObject *left_set = std::get_if<SetObject>(&_l),
                  *right_set = std::get_if<SetObject>(&_r);
        UnionExpr *left_un = std::get_if<UnionExpr>(&_l),
                  *right_un = std::get_if<UnionExpr>(&_r);
        if (left_un != nullptr) {
            return UnionExpr(std::make_shared<UnionExpr>(*left_un),
                             std::make_shared<IntervalExpr>(*right_inter));
        } else if (right_un != nullptr) {
            return UnionExpr(std::make_shared<IntervalExpr>(*left_inter),
                             std::make_shared<UnionExpr>(*right_un));
        } else if (left_un != nullptr && right_un != nullptr) {
            return UnionExpr(std::make_shared<UnionExpr>(*left_un),
                             std::make_shared<UnionExpr>(*right_un));
        } else if (left_inter != nullptr && right_inter != nullptr) {
            return UnionExpr(std::make_shared<IntervalExpr>(*left_inter),
                             std::make_shared<IntervalExpr>(*right_inter));
        } else if (left_set != nullptr && right_set != nullptr) {
            std::vector<ptr_t> set_elms = left_set->value;
            for (auto& e : right_set->value) {
                set_elms.push_back(e);
            }
            return visit(std::make_shared<SetObject>(set_elms));
        } else {
            m_Err("invalid use of 'union'");
        }
    }
    val_t m_VisitInterSectionExpr(InterSectionExpr* iun) {
        val_t _l = visit(iun->lhs), _r = visit(iun->rhs);
        IntervalExpr *left_inter = std::get_if<IntervalExpr>(&_l),
                     *right_inter = std::get_if<IntervalExpr>(&_r);
        SetObject *left_set = std::get_if<SetObject>(&_l),
                  *right_set = std::get_if<SetObject>(&_r);
        InterSectionExpr *left_un = std::get_if<InterSectionExpr>(&_l),
                         *right_un = std::get_if<InterSectionExpr>(&_r);
        if (left_un != nullptr) {
            return InterSectionExpr(
                std::make_shared<InterSectionExpr>(*left_un),
                std::make_shared<IntervalExpr>(*right_inter));
        } else if (right_un != nullptr) {
            return InterSectionExpr(
                std::make_shared<IntervalExpr>(*left_inter),
                std::make_shared<InterSectionExpr>(*right_un));
        } else if (left_un != nullptr && right_un != nullptr) {
            return InterSectionExpr(
                std::make_shared<InterSectionExpr>(*left_un),
                std::make_shared<InterSectionExpr>(*right_un));
        } else if (left_inter != nullptr && right_inter != nullptr) {
            return InterSectionExpr(
                std::make_shared<IntervalExpr>(*left_inter),
                std::make_shared<IntervalExpr>(*right_inter));
        } else if (left_set != nullptr && right_set != nullptr) {
            std::vector<ptr_t> set_elms;
            std::vector<Number> temp_elms;
            for (auto& e : right_set->value) {
                auto n_v = visit(e);
                m_CheckOrErr(std::get_if<Number>(&n_v) != nullptr,
                             "set can only contains numbers");
                temp_elms.push_back(*std::get_if<Number>(&n_v));
            }
            for (auto& e : left_set->value) {
                auto n_v = visit(e);
                m_CheckOrErr(std::get_if<Number>(&n_v) != nullptr,
                             "set can only contains numbers");
                if (std::find(temp_elms.begin(), temp_elms.end(),
                              *std::get_if<Number>(&n_v)) != temp_elms.end()) {
                    set_elms.push_back(e);
                }
            }
            return visit(std::make_shared<SetObject>(set_elms));
        } else {
            m_Err("invalid use of 'intersection'");
        }
    }
    val_t m_VisitSet(SetObject* so) {
        if (!so->value.empty()) {
            std::vector<ptr_t> content = so->value;
            std::set<Number> numbers;
            for (auto& e : content) {
                auto n_v = visit(e);
                m_CheckOrErr(std::get_if<Number>(&n_v) != nullptr,
                             "set can only contains numbers");
                numbers.insert(*std::get_if<Number>(&n_v));
            }
            content.clear();
            for (auto& e : numbers) {
                content.push_back(std::make_shared<Number>(e.val));
            }
            return SetObject(content);
        } else {
            return SetObject(so->value);
        }
    }
    val_t m_VisitVector(Vector* vec) {
        m_CheckOrErr((vec->value.size() == 2) || (vec->value.size() == 3),
                     "vector must have at least 2 elements");
        for (auto& e : vec->value) {
            val_t t_f_v_num = visit(e);
            m_CheckOrErr(std::get_if<Number>(&t_f_v_num) != nullptr,
                         "vectors can only contain numbers");
        }
        return Vector(vec->value);
    }
    val_t m_VisitSliceExpr(SliceExpr* sexpr) {
        val_t v_target = visit(sexpr->target);
        val_t v_num = visit(sexpr->index);
        SetObject* target = std::get_if<SetObject>(&v_target);
        m_CheckOrErr(target != nullptr,
                     "subscript expression is valid only for sets");
        Number* num = std::get_if<Number>(&v_num);
        m_CheckOrErr(num != nullptr, "invalid type");
        m_CheckOrErr((num->val < target->value.size()) && (num->val >= 0),
                     "invalid index");
        return visit(target->value.at(num->val));
    }
    val_t m_VisitEqualsSet(SetObject* left_set, SetObject* right_set) {
        m_CheckOrErr(left_set->value.size() == right_set->value.size(),
                     "compared sets must have the same size");
        for (std::size_t i = 0; i < left_set->value.size(); ++i) {
            m_CheckOrErr(
                right_set->value.at(i)->type() == left_set->value.at(i)->type(),
                "compared sets must have the same type");
            val_t v_left_num = visit(left_set->value.at(i));
            val_t v_right_num = visit(right_set->value.at(i));
            Number* left_num = std::get_if<Number>(&v_left_num);
            Number* right_num = std::get_if<Number>(&v_right_num);
            SetObject* t_left_set = std::get_if<SetObject>(&v_left_num);
            SetObject* t_right_set = std::get_if<SetObject>(&v_right_num);
            if ((left_num != nullptr) && (right_num != nullptr)) {
                if (left_num->val != right_num->val) return Boolean(false);
            }
            if ((t_left_set != nullptr) && (t_right_set != nullptr)) {
                if (t_left_set->value != t_right_set->value)
                    return Boolean(false);
            }
        }
        return Boolean(true);
    }

   public:
    Interpreter(const ami::exceptions::ExceptionInterface& ei) : ei(ei){};
    val_t visit(ptr_t expr) {
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
            case AstType::Comparison: {
                Comparison* comp = static_cast<Comparison*>(expr.get());
                switch (comp->op) {
                    default:
                        m_Err("invalid comparison operator");
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
                    case Op::NotEquals:
                        return m_VisitNotEquals(comp);
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
            case AstType::NotExpr: {
                return m_VisitNot(static_cast<NotExpr*>(expr.get()));
            }
            case AstType::NullExpr: {
                return m_VisitNull();
            }
            case AstType::Interval: {
                return m_VisitInterval(static_cast<IntervalExpr*>(expr.get()));
            }
            case AstType::InExpr: {
                return m_VisitInExpr(static_cast<InExpr*>(expr.get()));
            }
            case AstType::UnionExpr: {
                return m_VisitUnionExpr(static_cast<UnionExpr*>(expr.get()));
            }
            case AstType::IntersectionExpr: {
                return m_VisitInterSectionExpr(
                    static_cast<InterSectionExpr*>(expr.get()));
            }
            case AstType::SetObject: {
                return m_VisitSet(static_cast<SetObject*>(expr.get()));
            }
            case AstType::OpAndAssign: {
                return m_VisitOpAndAssignExpr(
                    static_cast<OpAndAssignExpr*>(expr.get()));
            }
            case AstType::SliceExpr: {
                return m_VisitSliceExpr(static_cast<SliceExpr*>(expr.get()));
            }
            case AstType::Vector: {
                return m_VisitVector(static_cast<Vector*>(expr.get()));
            }
        }
    }
    val_t visitvec(const std::vector<ptr_t>& exprs) {
        val_t out = visit(exprs.at(0));
        for (auto& elm : exprs) out = visit(elm);
        return out;
    }
    ~Interpreter() = default;
};
}  // namespace ami
