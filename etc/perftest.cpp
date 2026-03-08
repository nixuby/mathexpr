// perfomance test

#include "../src/mathexpr.h"

#include <chrono>
#include <format>
#include <print>

std::string format_duration(std::chrono::nanoseconds ns) {
    using namespace std::chrono;
    double count = static_cast<double>(ns.count());
    if (count < 1'000) {
        return std::format("{} ns", count);
    } else if (count < 1'000'000) {
        return std::format("{:.2f} us", count / 1'000);
    } else if (count < 1'000'000'000) {
        return std::format("{:.2f} ms", count / 1'000'000);
    } else {
        return std::format("{:.2f} s", count / 1'000'000'000);
    }
}

inline int constexpr ITERATIONS = 1'000'000;

void test(std::string_view name, std::function<uint64_t()> fn) {
    std::print("- Test \"{}\": ", name);
    auto start = std::chrono::high_resolution_clock::now();
    uint64_t result = fn();  // store result to prevent optimization
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = end - start;
    std::println("evaluated in {} ({} per evaluation) // Result: {}", format_duration(elapsed),
                 format_duration(elapsed / ITERATIONS), result);
}

// === Arithmetic: simple binary ===

uint64_t test_add_constants() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("2 + 2");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_sub_constants() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("100 - 37");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_mul_constants() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("6 * 7");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_div_constants() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("355 / 113");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_pow_operator() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("2 ** 10");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Arithmetic: chained / complex ===

uint64_t test_chained_add() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_mixed_arithmetic() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("3 + 4 * 2 / (1 - 5) ** 2");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_nested_parens() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("((((1 + 2) * 3) - 4) / 5)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Variables ===

uint64_t test_single_variable() {
    mathexpr::Context ctx;
    ctx.define("x", 42);
    mathexpr::Expression expr = ctx.compile("x");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_variable_arithmetic() {
    mathexpr::Context ctx;
    ctx.define("x", 10);
    ctx.define("y", 20);
    ctx.define("z", 30);
    mathexpr::Expression expr = ctx.compile("x * y + z");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_quadratic() {
    mathexpr::Context ctx;
    ctx.define("a", 1);
    ctx.define("b", -3);
    ctx.define("c", 2);
    ctx.define("x", 5);
    mathexpr::Expression expr = ctx.compile("a * x ** 2 + b * x + c");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Unary operators ===

uint64_t test_negation() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("-(-(-42))");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_logical_not() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("!0 + !1 + !!5");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_bitwise_not() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("~255");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Builtin functions: unary ===

uint64_t test_sin() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("sin(1.5)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_sqrt() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("sqrt(144)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_chained_functions() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("abs(sin(cos(1.0)))");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Builtin functions: binary ===

uint64_t test_pow_function() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("pow(2, 10)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_min_max() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("max(min(10, 20), 15)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_atan2() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("atan2(1, 1)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Custom functions ===

uint64_t test_custom_fn3() {
    mathexpr::Context ctx;
    ctx.define("x", 1);
    ctx.define("y", 2);
    ctx.define("z", 3);
    ctx.define("fn", [](double x, double y, double z) { return x * y + z; });
    mathexpr::Expression expr = ctx.compile("fn(x, y, z)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_custom_fn0() {
    mathexpr::Context ctx;
    ctx.define("rng", []() { return 42.0; });
    mathexpr::Expression expr = ctx.compile("rng()");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_custom_fn5() {
    mathexpr::Context ctx;
    ctx.define("blend", [](double a, double b, double c, double d, double e) { return (a + b + c + d + e) / 5.0; });
    mathexpr::Expression expr = ctx.compile("blend(1, 2, 3, 4, 5)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Ternary operator ===

uint64_t test_ternary_simple() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("1 ? 42 : 0");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_ternary_nested() {
    mathexpr::Context ctx;
    ctx.define("x", 5);
    mathexpr::Expression expr = ctx.compile("x > 10 ? 1 : x > 3 ? 2 : 3");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Boolean / logical operators ===

uint64_t test_logical_and_or() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("1 && 1 || 0 && 0");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_logical_complex() {
    mathexpr::Context ctx;
    ctx.define("a", 5);
    ctx.define("b", 10);
    ctx.define("c", 3);
    mathexpr::Expression expr = ctx.compile("(a > 3 && b < 20) || !(c == 3)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Comparison operators ===

uint64_t test_comparisons() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("(3 < 5) + (5 > 3) + (3 <= 3) + (5 >= 5) + (1 == 1) + (1 != 2)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Bitwise operators ===

uint64_t test_bitwise_and_or_xor() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("(255 & 15) | (3 ^ 5)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_bitwise_shifts() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("(1 << 10) >> 3");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Constants ===

uint64_t test_builtin_constants() {
    mathexpr::Context ctx;
    mathexpr::Expression expr = ctx.compile("pi * e");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Complex / realistic expressions ===

uint64_t test_distance_formula() {
    mathexpr::Context ctx;
    ctx.define("x1", 1);
    ctx.define("y1", 2);
    ctx.define("x2", 4);
    ctx.define("y2", 6);
    mathexpr::Expression expr = ctx.compile("sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_sigmoid() {
    mathexpr::Context ctx;
    ctx.define("x", 2.5);
    mathexpr::Expression expr = ctx.compile("1 / (1 + exp(-x))");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

uint64_t test_long_expression() {
    mathexpr::Context ctx;
    ctx.define("a", 1);
    ctx.define("b", 2);
    ctx.define("c", 3);
    ctx.define("d", 4);
    mathexpr::Expression expr = ctx.compile(
        "sin(a) + cos(b) * sqrt(c) - abs(d) + min(a, b) + max(c, d) + pow(a, 2) + ln(c) + floor(3.7) + ceil(2.1)");
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) result += expr.evaluate();
    return result;
}

// === Compilation benchmarks ===

uint64_t test_compile_simple() {
    mathexpr::Context ctx;
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) {
        result += ctx.evaluate("2 + 2");
    }
    return result;
}

uint64_t test_compile_complex() {
    mathexpr::Context ctx;
    ctx.define("x", 5);
    uint64_t result = 0;
    for (int i = 0; i < ITERATIONS; ++i) {
        result += ctx.evaluate("sin(x) + cos(x) * sqrt(x) + x ** 2");
    }
    return result;
}

int main() {
    std::println("=== Arithmetic: simple binary ===");
    test("2 + 2", test_add_constants);
    test("100 - 37", test_sub_constants);
    test("6 * 7", test_mul_constants);
    test("355 / 113", test_div_constants);
    test("2 ** 10", test_pow_operator);

    std::println("\n=== Arithmetic: chained / complex ===");
    test("1+2+3+...+10", test_chained_add);
    test("3+4*2/(1-5)**2", test_mixed_arithmetic);
    test("((((1+2)*3)-4)/5)", test_nested_parens);

    std::println("\n=== Variables ===");
    test("x", test_single_variable);
    test("x*y+z", test_variable_arithmetic);
    test("ax^2+bx+c (quadratic)", test_quadratic);

    std::println("\n=== Unary operators ===");
    test("-(-(-42))", test_negation);
    test("!0+!1+!!5", test_logical_not);
    test("~255", test_bitwise_not);

    std::println("\n=== Builtin functions: unary ===");
    test("sin(1.5)", test_sin);
    test("sqrt(144)", test_sqrt);
    test("abs(sin(cos(1.0)))", test_chained_functions);

    std::println("\n=== Builtin functions: binary ===");
    test("pow(2, 10)", test_pow_function);
    test("max(min(10,20),15)", test_min_max);
    test("atan2(1, 1)", test_atan2);

    std::println("\n=== Custom functions ===");
    test("fn(x,y,z) [3-arg]", test_custom_fn3);
    test("rng() [0-arg]", test_custom_fn0);
    test("blend(1..5) [5-arg]", test_custom_fn5);

    std::println("\n=== Ternary operator ===");
    test("1 ? 42 : 0", test_ternary_simple);
    test("nested ternary", test_ternary_nested);

    std::println("\n=== Boolean / logical ===");
    test("1&&1||0&&0", test_logical_and_or);
    test("(a>3&&b<20)||!(c==3)", test_logical_complex);

    std::println("\n=== Comparison operators ===");
    test("all 6 comparisons", test_comparisons);

    std::println("\n=== Bitwise operators ===");
    test("(255&15)|(3^5)", test_bitwise_and_or_xor);
    test("(1<<10)>>3", test_bitwise_shifts);

    std::println("\n=== Constants ===");
    test("pi * e", test_builtin_constants);

    std::println("\n=== Complex / realistic ===");
    test("distance formula", test_distance_formula);
    test("sigmoid(x)", test_sigmoid);
    test("10-func expression", test_long_expression);

    std::println("\n=== Compile + evaluate (no pre-compile) ===");
    test("compile+eval: 2+2", test_compile_simple);
    test("compile+eval: complex", test_compile_complex);
}