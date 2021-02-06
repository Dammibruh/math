#pragma once
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "builtins.hpp"
#include "parser.hpp"

namespace ami {
namespace scope {
static std::unordered_map<std::string, double> userdefined;
static std::unordered_map<std::string, ami::Function> userdefined_functions;
}  // namespace scope
class Interpreter {
    using u_ptr = std::shared_ptr<Expr>;
    std::unordered_map<std::string, double> arguments_scope;
    std::size_t call_count = 0;
    std::size_t max_call_count = 1'000;
    Number m_VisitAdd(BinaryOpExpr* boe) {
        return Number(visit((boe->lhs)).val + visit((boe->rhs)).val);
    }
    Number m_VisitSub(BinaryOpExpr* boe) {
        return Number(visit((boe->lhs)).val - visit((boe->rhs)).val);
    }
    Number m_VisitDiv(BinaryOpExpr* boe) {
        return Number(visit((boe->lhs)).val / visit((boe->rhs)).val);
    }
    Number m_VisitMult(BinaryOpExpr* boe) {
        return Number(visit((boe->lhs)).val * visit((boe->rhs)).val);
    }
    Number m_VisitIdent(Identifier* ident) {
        bool is_negative = (ident->name.at(0) == '-');
        std::string name = ident->name;
        bool is_a_function_arg =
            arguments_scope.find(name) != arguments_scope.end();
        bool is_a_defined_ident =
            scope::userdefined.find(name) != scope::userdefined.end();
        bool is_a_builtin_ident = ami::builtins::constants.find(name) !=
                                  ami::builtins::constants.end();
        if (is_negative && name.size() > 1) {
            name = std::string(name.begin() + 1, name.end());
        }
        if (is_a_function_arg) {
            return Number(arguments_scope.at(name));
        } else if (is_a_builtin_ident) {
            if (is_negative)
                return Number(-(ami::builtins::constants.at(name)));
            else
                return Number(ami::builtins::constants.at(name));
        } else if ((!scope::userdefined.empty()) && is_a_defined_ident) {
            if (is_negative)
                return Number(-scope::userdefined.at(name));
            else
                return Number(scope::userdefined.at(name));
        } else {
            throw std::runtime_error(
                std::string("use of undeclared identifier ") + name);
        }
    }
    Number m_VisitUserDefinedIdentifier(UserDefinedIdentifier* udi) {
        bool is_builtin = ami::builtins::constants.find(udi->name) !=
                          ami::builtins::constants.end();
        if (is_builtin) {
            throw std::runtime_error("Can't assign to built-in identifier \"" +
                                     udi->name + "\"");
        } else {
            auto val = visit((udi->value)).val;
            auto name = std::string(udi->name);
            scope::userdefined[name] = val;
            std::stringstream ss;
            ss << name << " = " << val;
            throw std::runtime_error(ss.str());
        }
    }
    Number m_VisitFunction(FunctionCall* fc) {
        /*
         * TODO:
         * add a addFunction function and object instead of separating builtin
         * and userdefined fumctions ,basically copy v8 engine's
         * functionCallBackInfo
         * or keep Function object also split this fat boi function into small
         * functions
         * use variant for string and Number and a fmt/error function
         * keep code clean and documented, maybe use unique pointer for 500ns
         * performance gain ??????
         * */
        // recursion check, haha die segmentation fault
        call_count++;
        if (call_count >= max_call_count)
            throw std::runtime_error("reached maximum call range " +
                                     std::to_string(max_call_count) + " calls");
        bool is_negative = fc->name.at(0) == '-';
        std::string name =
            is_negative ? std::string(fc->name.begin() + 1, fc->name.end())
                        : fc->name;
        decltype(ami::builtins::functions)::iterator get_builtin =
            ami::builtins::functions.find(name);
        bool is_builtin = get_builtin != ami::builtins::functions.end();
        auto get_userdefined = scope::userdefined_functions.find(name);
        bool is_userdefined =
            get_userdefined != scope::userdefined_functions.end();
        // helper variables
        std::vector<std::shared_ptr<ami::Expr>> args = fc->arguments;
        if (is_builtin) {
            std::vector<Number> parsed_args;
            if (args.size() != get_builtin->second.args_count) {
                throw std::runtime_error(
                    "mismatched arguments for function call '" + name +
                    "', called with " + std::to_string(args.size()) +
                    " argument, expected " +
                    std::to_string(get_builtin->second.args_count));
            }
            // evaluate each passed argument
            std::for_each(args.begin(), args.end(),
                          [&parsed_args, this](const auto& arg) {
                              parsed_args.push_back(visit(arg));
                          });
            if (is_negative)
                return Number(-(get_builtin->second.callback(parsed_args)));
            else
                return Number(get_builtin->second.callback(parsed_args));
        } else if (is_userdefined) {
            if (args.size() != get_userdefined->second.arguments.size()) {
                throw std::runtime_error(
                    "mismatched arguments for function call '" + name +
                    "', called with " + std::to_string(args.size()) +
                    " argument, expected " +
                    std::to_string(get_userdefined->second.arguments.size()));
            }
            std::vector<std::shared_ptr<Expr>> fc_args =
                get_userdefined->second.arguments;
            for (decltype(fc_args)::size_type i = 0; i < fc_args.size(); i++) {
                Identifier* ident =
                    static_cast<Identifier*>(fc_args.at(i).get());
                std::string cname = ident->name;
                Number num = visit(args.at(i));
                arguments_scope[cname] = num.val;
            }
            std::shared_ptr<Expr> fc_body = get_userdefined->second.body;
            // the function's body is evaluated only when it's called
            Number out = visit(fc_body);
            if (is_negative)
                return Number(-out.val);
            else
                return out;

        } else {
            throw std::runtime_error("use of undeclared function '" + name +
                                     "'");
        }
    }
    Number m_VisitFunctionDef(Function* func) {
        std::string name = func->name;
        bool is_builtin = ami::builtins::functions.find(name) !=
                          ami::builtins::functions.end();
        if (is_builtin) {
            throw std::runtime_error("can't assign to built-in function '" +
                                     name + "'");
        } else {
            scope::userdefined_functions.insert_or_assign(
                name, Function(func->name, func->body, func->arguments));
            return Number(
                0);  // return std::variant for string messages
                     // instead of returning numbers cuz it's confusing and dumb
        }
    }
    Number m_VisitMod(BinaryOpExpr* boe) {
        return Number(std::fmod(visit((boe->lhs)).val, visit((boe->rhs)).val));
    }
    Number m_VisitPow(BinaryOpExpr* boe) {
        return Number(std::pow(visit((boe->lhs)).val, visit((boe->rhs)).val));
    }
    Number m_VisitNegative(NegativeExpr* ex) {
        return Number(-visit(ex->value).val);
    }
    Number m_VisitNumber(Number* num) { return Number(num->val); }

   public:
    Interpreter() = default;
    Number visit(u_ptr expr) {
        switch (expr->type()) {
            default: {
                throw std::runtime_error("invalid expression");
            }
            case AstType::BinaryOp: {
                BinaryOpExpr* bopexpr = static_cast<BinaryOpExpr*>(expr.get());
                switch (bopexpr->op) {
                    default:
                        throw std::runtime_error("invalid operator");
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
        }
    }
    void addConstant(const std::string& name, double val) {
        scope::userdefined[name] = val;
    }
    Number visitvec(const std::vector<u_ptr>& exprs) {
        Number out(0);
        for (auto& elm : exprs) out = visit(elm);
        return out;
    }
    ~Interpreter() = default;
};
}  // namespace ami
