// a simple mathexpr REPL
// "quit" - to exit
// "def <name> <value>" - to define a variable with a numeric value
// "undef <name>" - to undefine a symbol (variable or function)
// "<expression>" - to evaluate an expression and print the result

#include "../src/mathexpr.h"

#include <format>
#include <iostream>
#include <print>
#include <span>

void execute(mathexpr::Context& ctx, std::vector<std::string> const& tokens) {
    std::string const& command = tokens[0];
    std::span<std::string const> arguments = std::span(tokens).subspan(1);

    if (command == "quit") {
        std::exit(0);
    } else if (command == "def") {
        if (arguments.size() != 2) {
            std::println("Usage: def <name> <value>");
            return;
        }
        std::string const& name = arguments[0];
        double value = std::stod(arguments[1]);
        ctx.define(name, value);
        std::println("Variable '{}' has been defined with value {}", name, value);
    } else if (command == "undef") {
        if (arguments.size() != 1) {
            std::println("Usage: undef <name>");
            return;
        }
        ctx.undefine(arguments[0]);
        std::println("Symbol '{}' has been undefined", arguments[0]);
    } else {
        std::string expression = command;
        for (auto const& arg : arguments) {
            expression += ' ';
            expression += arg;
        }

        try {
            double result = ctx.evaluate(expression);
            std::println("{}", result);
        } catch (std::exception const& e) {
            std::println("Error: {}", e.what());
        }
    }
}

int main() {
    mathexpr::Context ctx;

    std::println("mathexpr REPL (library version {}.{}.{})", mathexpr::version::MAJOR, mathexpr::version::MINOR,
                 mathexpr::version::PATCH);

    while (true) {
        std::print("> ");
        std::string line;
        std::getline(std::cin, line);
        bool quote = false;

        std::vector<std::string> tokens;
        std::string current;
        for (char c : line) {
            if (c == '"') {
                quote = !quote;
                continue;
            }

            if (std::isspace(static_cast<unsigned char>(c)) && !quote) {
                if (!current.empty()) {
                    tokens.push_back(std::move(current));
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            tokens.push_back(std::move(current));
        }

        if (tokens.empty()) continue;

        execute(ctx, tokens);
    }
}