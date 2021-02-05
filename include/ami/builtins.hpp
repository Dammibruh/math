#pragma once
#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include "ast.hpp"

namespace ami {
namespace builtins {
namespace details {
using arg_t = std::vector<ami::Number>;
struct FunctionHandler {
    using func_t = double (*)(const arg_t&);
    std::size_t args_count;
    func_t callback;
    FunctionHandler(std::size_t args_count, func_t func)
        : args_count(args_count), callback(func) {}
};
double b_sqrt(const arg_t& args) { return std::sqrt(args.at(0).val); }
double b_sin(const arg_t& args) { return std::sin(args.at(0).val); }
double b_cos(const arg_t& args) { return std::cos(args.at(0).val); }
double b_tan(const arg_t& args) { return std::tan(args.at(0).val); }
double b_sinh(const arg_t& args) { return std::sinh(args.at(0).val); }
double b_cosh(const arg_t& args) { return std::cosh(args.at(0).val); }
double b_tanh(const arg_t& args) { return std::tanh(args.at(0).val); }
double b_log(const arg_t& args) { return std::log(args.at(0).val); }
double b_log10(const arg_t& args) { return std::log10(args.at(0).val); }
double b_log2(const arg_t& args) { return std::log2(args.at(0).val); }
double b_min(const arg_t& args) {
    return std::fmin(args.at(0).val, args.at(1).val);
}
double b_max(const arg_t& args) {
    return std::fmax(args.at(0).val, args.at(1).val);
}
double b_abs(const arg_t& args) { return std::abs(args.at(0).val); }
double b_round(const arg_t& args) { return std::round(args.at(0).val); }
double b_ceil(const arg_t& args) { return std::ceil(args.at(0).val); }
double b_floor(const arg_t& args) { return std::floor(args.at(0).val); }
double b_gcd(const arg_t& args) {
    double x = args.at(0).val;
    double y = args.at(1).val;
    arg_t r_args{Number(y), Number(std::fmod(x, y))};
    return !y ? x : b_gcd(r_args);
}
double b_lcm(const arg_t& args) {
    double x = args.at(0).val, y = args.at(1).val;
    return (x * y) / b_gcd(args);
}
}  // namespace details
std::map<std::string, double> constants{{"pi", M_PI},
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
};
}  // namespace builtins
}  // namespace ami
