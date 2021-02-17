#pragma once
#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <vector>
#ifdef AMI_USE_PYTHON
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;
py::scoped_interpreter guard{};
py::module_ math = py::module_::import("math");
py::object pyinf = math.attr("inf");
py::object pynan = math.attr("nan");
long double aminf = pyinf.cast<long double>();
long double amnan = pynan.cast<long double>();
#define AMI_INF aminf
#define AMI_NAN amnan
#else
#define AMI_INF INFINITY
#define AMI_NAN NAN
#endif

#include "ast.hpp"
#include "errors.hpp"
#include "types.hpp"
// this is shit just a complete mess trash, dumb code rewrite it

namespace ami {
namespace builtins {
namespace details {
struct FunctionHandler {
    using func_t = val_t (*)(const arg_t&);
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
void checkOrErr(bool t, const std::string& f) {
    if (!t) throw std::runtime_error(f);
}
val_t b_sqrt(const arg_t& args) {
    return Number(std::sqrt(to_number(args.at(0))));
}
val_t b_sin(const arg_t& args) {
    return Number(std::sin(to_number(args.at(0))));
}
val_t b_cos(const arg_t& args) {
    return Number(std::cos(to_number(args.at(0))));
}
val_t b_tan(const arg_t& args) {
    return Number(std::tan(to_number(args.at(0))));
}
val_t b_sinh(const arg_t& args) {
    return Number(std::sinh(to_number(args.at(0))));
}
val_t b_cosh(const arg_t& args) {
    return Number(std::cosh(to_number(args.at(0))));
}
val_t b_tanh(const arg_t& args) {
    return Number(std::tanh(to_number(args.at(0))));
}
val_t b_log(const arg_t& args) {
    return Number(std::log(to_number(args.at(0))));
}
val_t b_log10(const arg_t& args) {
    return Number(std::log10(to_number(args.at(0))));
}
val_t b_log2(const arg_t& args) {
    return Number(std::log2(to_number(args.at(0))));
}
val_t b_min(const arg_t& args) {
    return Number(std::fmin(to_number(args.at(0)), to_number(args.at(1))));
}
val_t b_max(const arg_t& args) {
    return Number(std::fmax(to_number(args.at(0)), to_number(args.at(1))));
}
val_t b_abs(const arg_t& args) {
    return Number(std::abs(to_number(args.at(0))));
}
val_t b_round(const arg_t& args) {
    return Number(std::round(to_number(args.at(0))));
}
val_t b_ceil(const arg_t& args) {
    return Number(std::ceil(to_number(args.at(0))));
}
val_t b_floor(const arg_t& args) {
    return Number(std::floor(to_number(args.at(0))));
}
val_t b_gcd(const arg_t& args) {
    double x = to_number(args.at(0));
    double y = to_number(args.at(1));
    arg_t r_args{Number(y), Number(std::fmod(x, y))};
    return !y ? Number(x) : (b_gcd(r_args));
}
val_t b_lcm(const arg_t& args) {
    double x = to_number(args.at(0)), y = to_number(args.at(1));
    return Number((x * y) / to_number(b_gcd(args)));
}
std::random_device rnd;
std::mt19937 gen{rnd()};
val_t b_rand(const arg_t& args) {
    std::uniform_real_distribution<> dist(to_number(args.at(0)),
                                          to_number(args.at(1)));
    return Number(dist(gen));
}
}  // namespace details
std::map<std::string, long double> constants{{"pi", M_PI},
                                             {"tau", M_PI * 2},
                                             {"inf", AMI_INF},
                                             {"nan", AMI_NAN},
                                             {"e", M_E}};
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
    {"random", details::FunctionHandler(2, details::b_rand)},
};
}  // namespace builtins
}  // namespace ami
