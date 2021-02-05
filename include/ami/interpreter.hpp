#pragma once
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "builtins.hpp"
#include "parser.hpp"

namespace ami {
namespace scope {
static std::map<std::string, double> userdefined;
}  // namespace scope
class Interpreter {
    using u_ptr = std::shared_ptr<Expr>;
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
        bool is_negative = (ident->name[0] == '-');
        std::string name = ident->name;
        if (is_negative && name.size() > 1) {
            name = std::string(name.begin() + 1, name.end());
        }
        if (ami::builtins::constants.find(name) !=
            ami::builtins::constants.end()) {
            if (is_negative)
                return Number(-(ami::builtins::constants.at(name)));
            else
                return Number(ami::builtins::constants.at(name));
        } else if ((!scope::userdefined.empty()) &&
                   (scope::userdefined.find(name) !=
                    scope::userdefined.end())) {
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
        bool is_negative = fc->name[0] == '-';
        std::string name =
            is_negative ? std::string(fc->name.begin() + 1, fc->name.end())
                        : fc->name;
        auto get_builtin = ami::builtins::functions.find(name);
        bool is_builtin = get_builtin != ami::builtins::functions.end();
        if (is_builtin) {
            std::vector<std::shared_ptr<ami::Expr>> args = fc->arguments;
            std::vector<Number> parsed_args;
            if (args.size() != get_builtin->second.args_count) {
                throw std::runtime_error(
                    "mismatched arguments for function call '" + name +
                    "', called with " + std::to_string(args.size()) +
                    " argument, expected " +
                    std::to_string(get_builtin->second.args_count));
            }
            std::for_each(args.begin(), args.end(),
                          [&parsed_args, this](auto& arg) {
                              parsed_args.push_back(visit(arg));
                          });
            if (is_negative)
                return Number(-(get_builtin->second.callback(parsed_args)));
            else
                return Number(get_builtin->second.callback(parsed_args));
        } else {
            throw std::runtime_error("use of undeclared function '" + name +
                                     "'");
        }
    }
    Number m_VisitMod(BinaryOpExpr* boe) {
        return Number(std::fmod(visit((boe->lhs)).val, visit((boe->rhs)).val));
    }
    Number m_VisitPow(BinaryOpExpr* boe) {
        return Number(std::pow(visit((boe->lhs)).val, visit((boe->rhs)).val));
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
        }
    }
    void addConstant(const std::string& name, double val) {
        scope::userdefined[name] = val;
    }
    ~Interpreter() = default;
};
}  // namespace ami
