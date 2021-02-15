#include <benchmark/benchmark.h>

#include <ami/ami.hpp>

static void NumberParsing(benchmark::State& state) {
    for (auto _ : state) {
        ami::Parser(ami::Lexer("0").lex(), "0", "null").parse();
    }
}
static void NumberLexing(benchmark::State& state) {
    for (auto _ : state) {
        ami::Lexer("0").lex();
    }
}
static void FunctionParsing(benchmark::State& state) {
    std::string expr{"f(x) -> x^2"};
    for (auto _ : state) {
        ami::Parser(ami::Lexer(expr).lex(), expr, "null").parse();
    }
}
static void SetParsing(benchmark::State& state) {
    std::string expr{"{5, 9, 3, 739}"};
    for (auto _ : state) {
        ami::Parser(ami::Lexer(expr).lex(), expr, "null").parse();
    }
}
static void CompParsing(benchmark::State& state) {
    std::string expr{"0 <= 1"};
    for (auto _ : state) {
        ami::Parser(ami::Lexer(expr).lex(), expr, "null").parse();
    }
}

BENCHMARK(NumberParsing)->Range(0, 1 << 22);
BENCHMARK(NumberLexing)->Range(0, 1 << 22);
BENCHMARK(FunctionParsing)->Range(0, 1 << 22);
BENCHMARK(SetParsing)->Range(0, 1 << 22);
BENCHMARK(CompParsing)->Range(0, 1 << 22);
BENCHMARK_MAIN();
