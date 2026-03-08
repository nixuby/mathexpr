# mathexpr

Small header-only math expression evaluator for C++20

## Features

- Header-only, contained in a namespace, no external dependencies
- Expression parsing and evaluation
- Define variables and functions using `Context::define(name, value)`
- Built-in constants and functions: `pi`, `e`, `sin`, `cos`, `tan`, `sqrt`, `pow`, `log`, and more
- Operators: `+`, `-`, `*`, `/`, `^` (exponentiation)
- Parentheses and function-call syntax: `fun(1, 2, 3)`

## Example

```cpp
#include "mathexpr.h"
#include <iostream>

int main() {
	mathexpr::Context ctx;

	std::cout << ctx.evaluate("2 + 2") << "\n";            // 4
	std::cout << ctx.evaluate("sin(pi / 2)") << "\n";      // 1

	ctx.define("x", 3.0);
	std::cout << ctx.evaluate("2 + x") << "\n"; // 5

	ctx.define("fun", [](mathexpr::Number a, mathexpr::Number b, mathexpr::Number c) {
		return a + b - c;
	});
	std::cout << ctx.evaluate("fun(1, 2, 4)") << "\n";     // -1
	std::cout << ctx.evaluate("2 ^ 3 ^ 2") << "\n";        // 512
}
```

## Notes

- `evaluate()` throws `std::runtime_error` on parse/evaluation errors
- Exponentiation is done right-to-left (i.e. `2^3^4 == 2^(3^4)`)

## Todo

- Boolean logic: `true`, `false`, operators `&&`, `||`, `!`, comparison operators `==`, `!=`, `<`, `>`, `<=`, `>=`, ternary operator `? :`
- Bitwise operators: `&`, `|`, `^`, `~`, `<<`, `>>`
- Precompile expressions for better evaluation performance