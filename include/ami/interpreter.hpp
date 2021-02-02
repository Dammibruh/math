#pragma once
#include <cmath>
#include <iostream>
#include <memory>

#include "parser.hpp"

namespace ami {
class Interpreter {
    using u_ptr = std::unique_ptr<Expr>;
    std::map<std::string, double> builtin{{"pi", M_PI},
                                          {"eu", M_E},
                                          {"tau", M_PI * 2},
                                          {"inf", INFINITY},
                                          {"nan", NAN}};
    std::map<std::string, double>* userdefined;
    Number m_VisitAdd(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val +
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitSub(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val -
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitDiv(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val /
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitMult(BinaryOpExpr* boe) {
        return Number(visit(std::move(boe->lhs)).val *
                      visit(std::move(boe->rhs)).val);
    }
    Number m_VisitIdent(Identifier* ident) {
        bool is_negative = ident->name[0] == '-';
        std::string name;
        if (is_negative) {
            name = std::string(ident->name.begin() + 1, ident->name.end());
        }
        if (builtin.find(name) != builtin.end()) {
            if (is_negative)
                return Number(-(builtin.at(name)));
            else
                return Number(builtin.at(name));
        } else if (userdefined != nullptr &&
                   userdefined->find(name) != userdefined->end()) {
            std::cout << "user defined\n";
            if (is_negative)
                return Number(-userdefined->at(name));
            else
                return Number(userdefined->at(name));
        } else {
            throw std::runtime_error(
                std::string("use of undeclared identifier ") + name);
        }
    }
    Number m_VisitUserDefinedIdentifier(UserDefinedIdentifier* udi) {
        bool is_builtin = builtin.find(udi->name) != builtin.end();
        std::cout << "defining an ident\n";
        if (is_builtin) {
            throw std::runtime_error("Can't assign to built-in identifier \"" +
                                     udi->name + "\"");
        } else if (userdefined != nullptr) {
            (*userdefined)[std::move(udi->name)] =
                visit(std::move(udi->value)).val;
            return Number(0);
        }
    }
    Number m_VisitMod(BinaryOpExpr* boe) {
        return Number(std::fmod(visit(std::move(boe->lhs)).val,
                                visit(std::move(boe->rhs)).val));
    }
    Number m_VisitPow(BinaryOpExpr* boe) {
        return Number(std::pow(visit(std::move(boe->lhs)).val,
                               visit(std::move(boe->rhs)).val));
    }
    Number m_VisitNumber(Number* num) { return Number(num->val); }

   public:
    explicit Interpreter(std::map<std::string, double>* scope = nullptr)
        : userdefined(scope) {}
    Number visit(u_ptr expr) {
        switch (expr->type()) {
            default: {
                throw std::runtime_error("invalid expr");
            }
            case AstType::BinaryOp: {
                BinaryOpExpr* bopexpr =
                    static_cast<BinaryOpExpr*>(std::move(expr).get());
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
                return Number(static_cast<Number*>(expr.get())->val);
            }
            case AstType::Identifier: {
                return m_VisitIdent(static_cast<Identifier*>(expr.get()));
            }
            case AstType::UserDefinedIdentifier: {
                return m_VisitUserDefinedIdentifier(
                    static_cast<UserDefinedIdentifier*>(expr.get()));
            }
        }
    }
};
}  // namespace ami
