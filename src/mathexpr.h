// "mathexpr" v0.1 (c) 2026 Nixuby (https://nixuby.com)
// License: MIT (https://opensource.org/license/MIT)
// Math expression evaluation library
// C++20, header-only, no dependencies

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <numbers>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace mathexpr {
    namespace version {
        inline int constexpr ID = 0;
        inline int constexpr MAJOR = 0;
        inline int constexpr MINOR = 1;
        inline int constexpr PATCH = 0;
    }

    using Number = double;  // Type used for numbers in expressions

    namespace detail {
        template <std::size_t>
        using AlwaysNumber = Number;

        // Arity is the number of arguments in a callable type

        // Not a callable type -- the `value` is absent
        template <class T, class = void>
        struct arity {};

        template <class T>
        struct arity<T, std::void_t<decltype(&T::operator())>> : arity<decltype(&T::operator())> {};

        template <class C, class R, class... Args>
        struct arity<R (C::*)(Args...) const, void> {
            static constexpr std::size_t value = sizeof...(Args);
        };

        template <class C, class R, class... Args>
        struct arity<R (C::*)(Args...), void> {
            static constexpr std::size_t value = sizeof...(Args);
        };

        template <class R, class... Args>
        struct arity<R (*)(Args...), void> {
            static constexpr std::size_t value = sizeof...(Args);
        };
    }

    // A non-callable symbol with a defined numeric value
    class Variable {
    public:
        explicit Variable(std::string name, Number value) : name_(std::move(name)), value_(value) {}

        [[nodiscard]]
        std::string const& get_name() const {
            return name_;
        }

        [[nodiscard]]
        Number get_value() const {
            return value_;
        }

        void set_name(std::string name) { name_ = std::move(name); }

        void set_value(Number value) { value_ = value; }

    private:
        std::string name_ = "";
        Number value_ = 0.0;
    };

    template <std::size_t N, class = std::make_index_sequence<N>>
    class Function;

    // A callable symbol with a defined name and a function that takes N arguments
    template <std::size_t N, std::size_t... Is>
    class Function<N, std::index_sequence<Is...>> {
    public:
        using Callback = std::function<Number(detail::AlwaysNumber<Is>...)>;

        explicit Function(std::string name, Callback function)
            : name_(std::move(name)), function_(std::move(function)) {}

        [[nodiscard]]
        std::string const& get_name() const {
            return name_;
        }

        [[nodiscard]]
        Callback const& get_function() const {
            return function_;
        }

        [[nodiscard]]
        Number call(std::vector<Number> const& args) const {
            if (args.size() != N) {
                throw std::runtime_error("Function '" + name_ + "' expects " + std::to_string(N) +
                                         " argument(s), got " + std::to_string(args.size()));
            }
            return function_(args[Is]...);
        }

        void set_name(std::string name) { name_ = std::move(name); }

        void set_function(Callback function) { function_ = std::move(function); }

    private:
        std::string name_ = "";
        Callback function_;
    };

    // A sum type for variables and functions of different arities, used for symbol lookup
    using Symbol = std::variant<Variable, Function<0>, Function<1>, Function<2>, Function<3>, Function<4>, Function<5>>;

    inline std::string const& get_symbol_name(Symbol const& symbol) {
        return std::visit(
            [](auto const& s) -> std::string const& {
                using T = std::decay_t<decltype(s)>;
                if constexpr (std::is_same_v<T, Variable>) {
                    return s.get_name();
                } else {
                    return s.get_name();
                }
            },
            symbol);
    }

    // Context for expression evaluation and symbol management
    class Context {
    public:
        explicit Context(bool should_define_builtins = true) {
            if (should_define_builtins) {
                define_builtins();
            }
        }

        // Define a new variable or update an existing one
        void define(std::string name, Number value) {
            for (auto& sym : symbols_) {
                if (auto* var = std::get_if<Variable>(&sym)) {
                    if (var->get_name() == name) {
                        var->set_value(value);
                        return;
                    }
                }
            }
            symbols_.push_back(Variable(std::move(name), value));
        }

        // Define a new function or update an existing one
        template <class F>
            requires requires { detail::arity<std::decay_t<F>>::value; }
        void define(std::string name, F&& f) {
            constexpr std::size_t N = detail::arity<std::decay_t<F>>::value;
            static_assert(N <= 5, "Functions with more than 5 arguments are not supported");
            for (auto& sym : symbols_) {
                if (auto* func = std::get_if<Function<N>>(&sym)) {
                    if (func->get_name() == name) {
                        func->set_function(std::forward<F>(f));
                        return;
                    }
                }
            }
            symbols_.push_back(Function<N>(std::move(name), std::forward<F>(f)));
        }

        // Undefine a symbol by name
        void undefine(std::string_view name) {
            std::ranges::remove_if(symbols_, [&](Symbol const& sym) { return get_symbol_name(sym) == name; });
        }

        [[nodiscard]]
        Number evaluate(std::string_view input) const {
            thread_local static std::vector<Token> tokens;
            tokens.clear();
            tokenize(tokens, input);
            Parser parser(tokens, symbols_);
            return parser.parse();
        }

        std::vector<Symbol> const& get_symbols() const { return symbols_; }

        Symbol const& get_symbol(std::string_view name) const {
            for (auto const& sym : symbols_) {
                if (get_symbol_name(sym) == name) {
                    return sym;
                }
            }
            throw std::runtime_error("Unknown symbol: '" + std::string(name) + "'");
        }

    private:
        enum class TokenType { NumberLiteral, Identifier, Plus, Minus, Star, Slash, Caret, LParen, RParen, Comma, End };

        struct Token {
            TokenType type;
            std::string text;
            Number value = 0;
        };

        class Parser {
        public:
            explicit Parser(std::vector<Token> const& tokens, std::vector<Symbol> const& symbols)
                : tokens_(tokens), symbols_(symbols) {}

            Token const& peek() const { return tokens_[pos_]; }
            Token const& advance() { return tokens_[pos_++]; }
            bool match(TokenType t) {
                if (peek().type == t) {
                    advance();
                    return true;
                }
                return false;
            }
            void expect(TokenType t, char const* msg) {
                if (!match(t)) throw std::runtime_error(msg);
            }

            Number parse() {
                auto result = expression();
                if (peek().type != TokenType::End) throw std::runtime_error("Unexpected token: '" + peek().text + "'");
                return result;
            }

            Number expression() {
                auto left = term();
                while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
                    if (advance().type == TokenType::Plus) left += term();
                    else left -= term();
                }
                return left;
            }

            Number term() {
                auto left = exponent();
                while (peek().type == TokenType::Star || peek().type == TokenType::Slash) {
                    if (advance().type == TokenType::Star) left *= exponent();
                    else left /= exponent();
                }
                return left;
            }

            Number exponent() {
                auto base = unary();
                if (match(TokenType::Caret)) return std::pow(base, exponent());  // recurse for right-associativity
                return base;
            }

            Number unary() {
                if (peek().type == TokenType::Minus) {
                    advance();
                    return -unary();
                }
                if (peek().type == TokenType::Plus) {
                    advance();
                    return unary();
                }
                return primary();
            }

            Number primary() {
                if (peek().type == TokenType::NumberLiteral) return advance().value;
                if (peek().type == TokenType::Identifier) {
                    auto name = advance().text;
                    if (peek().type == TokenType::LParen) {
                        advance();
                        std::vector<Number> args;
                        if (peek().type != TokenType::RParen) {
                            args.push_back(expression());
                            while (match(TokenType::Comma)) args.push_back(expression());
                        }
                        expect(TokenType::RParen, "Expected ')'");
                        return call_function(name, args);
                    }
                    return lookup_variable(name);
                }
                if (peek().type == TokenType::LParen) {
                    advance();
                    auto result = expression();
                    expect(TokenType::RParen, "Expected ')'");
                    return result;
                }
                throw std::runtime_error("Unexpected token: '" + peek().text + "'");
            }

            Number lookup_variable(std::string const& name) const {
                for (auto const& sym : symbols_) {
                    std::string const& sym_name = get_symbol_name(sym);
                    if (sym_name == name) {
                        if (auto* var = std::get_if<Variable>(&sym)) {
                            return var->get_value();
                        } else {
                            throw std::runtime_error("Symbol '" + name + "' is a function");
                        }
                    }
                }
                throw std::runtime_error("Unknown variable: '" + name + "'");
            }

            Number call_function(std::string const& name, std::vector<Number> const& args) const {
                for (auto const& sym : symbols_) {
                    std::string const& sym_name = get_symbol_name(sym);
                    if (sym_name != name) continue;

                    char constexpr VAR = 0x0;
                    char constexpr FUNC = 0x1;
                    char constexpr NONE = 0x2;

                    auto result = std::visit(
                        [&](auto const& s) -> std::pair<char, Number> {
                            using T = std::decay_t<decltype(s)>;
                            if constexpr (std::is_same_v<T, Variable>) {
                                return { VAR, 0 };
                            } else {
                                if (s.get_name() == name) return { FUNC, s.call(args) };
                                return { NONE, 0 };
                            }
                        },
                        sym);

                    if (result.first == FUNC) {
                        return result.second;
                    } else if (result.first == VAR) {
                        throw std::runtime_error("Symbol '" + name + "' is not callable");
                    }
                }
                throw std::runtime_error("Unknown function: '" + name + "'");
            }

        private:
            std::vector<Token> const& tokens_;
            std::vector<Symbol> const& symbols_;
            std::size_t pos_ = 0;
        };

        static void tokenize(std::vector<Token>& tokens, std::string_view input) {
            std::size_t i = 0;
            while (i < input.size()) {
                if (std::isspace(static_cast<unsigned char>(input[i]))) {
                    ++i;
                    continue;
                }
                if (std::isdigit(static_cast<unsigned char>(input[i])) || input[i] == '.') {
                    std::size_t start = i;
                    while (i < input.size() && (std::isdigit(static_cast<unsigned char>(input[i])) || input[i] == '.'))
                        ++i;
                    if (i < input.size() && (input[i] == 'e' || input[i] == 'E')) {
                        ++i;
                        if (i < input.size() && (input[i] == '+' || input[i] == '-')) ++i;
                        while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
                    }
                    std::string text(input.substr(start, i - start));
                    tokens.push_back({ TokenType::NumberLiteral, text, std::stod(text) });
                    continue;
                }
                if (std::isalpha(static_cast<unsigned char>(input[i])) || input[i] == '_') {
                    std::size_t start = i;
                    while (i < input.size() && (std::isalnum(static_cast<unsigned char>(input[i])) || input[i] == '_'))
                        ++i;
                    tokens.push_back({ TokenType::Identifier, std::string(input.substr(start, i - start)) });
                    continue;
                }
                switch (input[i]) {
                    case '+':
                        tokens.push_back({ TokenType::Plus, "+" });
                        break;
                    case '-':
                        tokens.push_back({ TokenType::Minus, "-" });
                        break;
                    case '*':
                        tokens.push_back({ TokenType::Star, "*" });
                        break;
                    case '/':
                        tokens.push_back({ TokenType::Slash, "/" });
                        break;
                    case '^':
                        tokens.push_back({ TokenType::Caret, "^" });
                        break;
                    case '(':
                        tokens.push_back({ TokenType::LParen, "(" });
                        break;
                    case ')':
                        tokens.push_back({ TokenType::RParen, ")" });
                        break;
                    case ',':
                        tokens.push_back({ TokenType::Comma, "," });
                        break;
                    default:
                        throw std::runtime_error(std::string("Unexpected character: '") + input[i] + "'");
                }
                ++i;
            }
            tokens.push_back({ TokenType::End, "" });
        }

        void define_builtins() {
            // Constants
            define("pi", std::numbers::pi);
            define("e", std::numbers::e);

            // Unary functions
            define("sin", [](Number x) { return std::sin(x); });
            define("cos", [](Number x) { return std::cos(x); });
            define("tan", [](Number x) { return std::tan(x); });
            define("asin", [](Number x) { return std::asin(x); });
            define("acos", [](Number x) { return std::acos(x); });
            define("atan", [](Number x) { return std::atan(x); });
            define("sqrt", [](Number x) { return std::sqrt(x); });
            define("cbrt", [](Number x) { return std::cbrt(x); });
            define("abs", [](Number x) { return std::abs(x); });
            define("ln", [](Number x) { return std::log(x); });
            define("log2", [](Number x) { return std::log2(x); });
            define("log10", [](Number x) { return std::log10(x); });
            define("exp", [](Number x) { return std::exp(x); });
            define("floor", [](Number x) { return std::floor(x); });
            define("ceil", [](Number x) { return std::ceil(x); });
            define("round", [](Number x) { return std::round(x); });
            define("deg2rad", [](Number x) { return x * std::numbers::pi / 180; });
            define("rad2deg", [](Number x) { return x * 180 / std::numbers::pi; });

            // Binary functions
            define("pow", [](Number a, Number b) { return std::pow(a, b); });
            define("atan2", [](Number y, Number x) { return std::atan2(y, x); });
            define("min", [](Number a, Number b) { return std::min(a, b); });
            define("max", [](Number a, Number b) { return std::max(a, b); });
            define("log", [](Number base, Number x) { return std::log(x) / std::log(base); });
        }

        std::vector<Symbol> symbols_;
    };
}