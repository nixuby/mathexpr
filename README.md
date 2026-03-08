# mathexpr

Small header-only math expression evaluator for C++20

## Features

- Header-only, contained in a namespace, no external dependencies
- Expression parsing, compilation, and evaluation
- Define variables and functions using `Context::define(name, value)`
- Built-in constants and functions: `pi`, `e`, `true`, `false`, `sin`, `cos`, `tan`, `sqrt`, `pow`, `log`, and more
- Number operators: `+`, `-`, `*`, `/`, `**`
- Bitwise operators: `&`, `|`, `^`, `~`, `<<`, `>>`
- Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical operators: `&&`, `||`, `!`, with the ternary operator `?:`
- Parentheses and function-call syntax: `fun(1, 2, 3)`

## Example

```cpp
#include "mathexpr.h"
#include <iostream>

int main() {
    mathexpr::Context ctx;

    // Basic expressions and built-ins
    std::cout << ctx.evaluate("2 + 2") << "\n"; // = 4
    std::cout << ctx.evaluate("sin(pi / 2)") << "\n"; // = 1

    // Define a variable
    ctx.define("x", 3.0);
    std::cout << ctx.evaluate("2 + x") << "\n"; // = 5

    // Define a function
    ctx.define<3>("fun", [](auto a, auto b, auto c) { return a + b - c; });
    std::cout << ctx.evaluate("fun(1, 2, 4)") << "\n"; // = -1

    // Comparison and logical operators
    std::cout << ctx.evaluate("x > 2 ? 10 : 5") << "\n"; // = 10

    // Bitwise operators
    std::cout << ctx.evaluate("5 & 3") << "\n"; // = 1

    // Compilation for improved performance of repeated evaluations
    auto compiled = ctx.compile("x * x + 2 * x + 1");
    std::cout << compiled.evaluate() << "\n"; // = 16 (when x = 3)
}
```

## Notes

- `evaluate()` throws `std::runtime_error` on parse/evaluation errors
- Exponentiation is done right-to-left (i.e. `2^3^4 == 2^(3^4)`)