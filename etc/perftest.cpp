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

void test0() {
    mathexpr::Context ctx;

    int const I = 1'000'000;
    uint64_t result = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < I; ++i) {
        result += ctx.evaluate("2 + 2");
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::nanoseconds elapsed = end - start;
    std::println("\"2 + 2\": Result: {}, evaluated {} times in {} ({} per evaluation)", result, I,
                 format_duration(elapsed), format_duration(elapsed / I));
}

void test1() {
    mathexpr::Context ctx;

    int const I = 1'000'000;
    uint64_t result = 0;

    auto start = std::chrono::high_resolution_clock::now();
    ctx.define("x", 2);
    ctx.define("y", 3);
    ctx.define("z", 4);
    ctx.define("fn", [](double a, double b, double c) { return a + b * c; });
    for (int i = 0; i < I; ++i) {
        result += ctx.evaluate("fn(x, y, z)");
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::nanoseconds elapsed = end - start;
    std::println("\"fn(x, y, z)\": Result: {}, evaluated {} times in {} ({} per evaluation)", result, I,
                 format_duration(elapsed), format_duration(elapsed / I));
}

void test2() {
    mathexpr::Context ctx;

    int const I = 1'000'000;
    uint64_t result = 0;

    auto start = std::chrono::high_resolution_clock::now();
    ctx.define("x", 45);
    for (int i = 0; i < I; ++i) {
        result += ctx.evaluate("sin(x)");
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::nanoseconds elapsed = end - start;
    std::println("\"sin(x)\": Result: {}, evaluated {} times in {} ({} per evaluation)", result, I,
                 format_duration(elapsed), format_duration(elapsed / I));
}

int main() {
    test0();
    test1();
    test2();
}