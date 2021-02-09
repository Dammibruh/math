#pragma once
#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "ast.hpp"
#include "errors.hpp"

namespace ami {
namespace builtins {
namespace details {
using val_t = std::variant<ami::Number, ami::Boolean, std::string>;
using arg_t = std::vector<val_t>;
struct FunctionHandler {
    using func_t = double (*)(const arg_t&);
    std::size_t args_count;
    func_t callback;
    FunctionHandler(std::size_t args_count, func_t func)
        : args_count(args_count), callback(func) {}
};
double to_number(const val_t& a) {
    if (auto _get = std::get_if<Number>(&a))
        return _get->val;
    else
        throw std::runtime_error("expected number in function args");
}
double b_sqrt(const arg_t& args) { return std::sqrt(to_number(args.at(0))); }
double b_sin(const arg_t& args) { return std::sin(to_number(args.at(0))); }
double b_cos(const arg_t& args) { return std::cos(to_number(args.at(0))); }
double b_tan(const arg_t& args) { return std::tan(to_number(args.at(0))); }
double b_sinh(const arg_t& args) { return std::sinh(to_number(args.at(0))); }
double b_cosh(const arg_t& args) { return std::cosh(to_number(args.at(0))); }
double b_tanh(const arg_t& args) { return std::tanh(to_number(args.at(0))); }
double b_log(const arg_t& args) { return std::log(to_number(args.at(0))); }
double b_log10(const arg_t& args) { return std::log10(to_number(args.at(0))); }
double b_log2(const arg_t& args) { return std::log2(to_number(args.at(0))); }
double b_min(const arg_t& args) {
    return std::fmin(to_number(args.at(0)), to_number(args.at(1)));
}
double b_max(const arg_t& args) {
    return std::fmax(to_number(args.at(0)), to_number(args.at(1)));
}
double b_abs(const arg_t& args) { return std::abs(to_number(args.at(0))); }
double b_round(const arg_t& args) { return std::round(to_number(args.at(0))); }
double b_ceil(const arg_t& args) { return std::ceil(to_number(args.at(0))); }
double b_floor(const arg_t& args) { return std::floor(to_number(args.at(0))); }
double b_gcd(const arg_t& args) {
    double x = to_number(args.at(0));
    double y = to_number(args.at(1));
    arg_t r_args{Number(y), Number(std::fmod(x, y))};
    return !y ? x : b_gcd(r_args);
}
double b_lcm(const arg_t& args) {
    double x = to_number(args.at(0)), y = to_number(args.at(1));
    return (x * y) / b_gcd(args);
}
}  // namespace details
std::map<std::string, long double> constants{{"pi", M_PI},
                                             {"tau", M_PI * 2},
                                             {"inf", INFINITY},
                                             {"nan", NAN},
                                             {"eu", M_E}};
std::map<std::string, details::FunctionHandler> functions{
    {"sqrt", details::FunctionHandler(1, details::b_sqrt)},
    {"sin", details::FunctionHandler(1, details::b_sin)},
    {"cos", details::FunctionHandler(1, details::b_cos)},
    {"tan", details::FunctionHandler(1, details::b_tan)},
    {"sinh", details::FunctionHandler(1, details::b_sinh)},
    {"cosh", details::FunctionHandler(1, details::b_cosh)},
    {"tanh", details::FunctionHandler(1, details::b_tan)},
    {"log", details::FunctionHandler(1, details::b_log)},
    {"max", details::FunctionHandler(2, details::b_max)},
    {"min", details::FunctionHandler(2, details::b_min)},
    {"abs", details::FunctionHandler(1, details::b_abs)},
    {"round", details::FunctionHandler(1, details::b_round)},
    {"floor", details::FunctionHandler(1, details::b_floor)},
    {"ceil", details::FunctionHandler(1, details::b_ceil)},
    {"gcd", details::FunctionHandler(2, details::b_gcd)},
    {"lcm", details::FunctionHandler(2, details::b_lcm)},
    {"log10", details::FunctionHandler(1, details::b_log10)},
    {"log2", details::FunctionHandler(1, details::b_log2)},
};
}  // namespace builtins
}  // namespace ami
