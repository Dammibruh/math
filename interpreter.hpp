#pragma once
#include <cmath>

#include "parser.hpp"

class Interpreter {
    using u_ptr = std::unique_ptr<Expr>;
    std::map<std::string, double> builtin{{"pi", M_PI},
                                          {"eu", M_E},
                                          {"tau", M_PI * 2},
                                          {"inf", INFINITY},
                                          {"nan", NAN}};
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
        std::string name = is_negative
                               ? ident->name.substr(1, ident->name.size())
                               : ident->name;
        if (builtin.find(name) != builtin.end()) {
            if (is_negative)
                return Number(-builtin[name]);
            else
                return Number(builtin[name]);
        } else {
            throw std::runtime_error("use of undeclared identifier " + name);
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
    Interpreter() = default;
    Number visit(u_ptr expr) {
        switch (expr->type()) {
            default: {
                throw std::runtime_error("invalid expr");
                break;
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
                return m_VisitNumber(static_cast<Number*>(expr.get()));
            }
            case AstType::Identifier: {
                return m_VisitIdent(static_cast<Identifier*>(expr.get()));
            }
        }
    }
};
