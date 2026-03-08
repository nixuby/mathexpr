// "mathexpr" v0.2 (c) 2026 Nixuby (https://nixuby.com)
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
        inline int constexpr ID = 1;
        inline int constexpr MAJOR = 0;
        inline int constexpr MINOR = 2;
        inline int constexpr PATCH = 0;
    }

    using Number = double;  // Type used for numbers in expressions
    using Int = long long;  // Type used for bitwise operations

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

    namespace detail {
        enum class TokenType {
            NumberLiteral,
            Identifier,
            Plus,
            Minus,
            Star,
            StarStar,
            Slash,
            Caret,
            Ampersand,
            Pipe,
            Tilde,
            LShift,
            RShift,
            LParen,
            RParen,
            Comma,
            End
        };

        struct Token {
            TokenType type;
            std::string text;
            Number value = 0;
        };

        inline std::vector<Token> tokenize(std::string_view input) {
            std::vector<Token> tokens;
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
                        if (i + 1 < input.size() && input[i + 1] == '*') {
                            tokens.push_back({ TokenType::StarStar, "**" });
                            ++i;
                        } else {
                            tokens.push_back({ TokenType::Star, "*" });
                        }
                        break;
                    case '/':
                        tokens.push_back({ TokenType::Slash, "/" });
                        break;
                    case '^':
                        tokens.push_back({ TokenType::Caret, "^" });
                        break;
                    case '&':
                        tokens.push_back({ TokenType::Ampersand, "&" });
                        break;
                    case '|':
                        tokens.push_back({ TokenType::Pipe, "|" });
                        break;
                    case '~':
                        tokens.push_back({ TokenType::Tilde, "~" });
                        break;
                    case '<':
                        if (i + 1 < input.size() && input[i + 1] == '<') {
                            tokens.push_back({ TokenType::LShift, "<<" });
                            ++i;
                        } else {
                            throw std::runtime_error(std::string("Unexpected character: '") + input[i] + "'");
                        }
                        break;
                    case '>':
                        if (i + 1 < input.size() && input[i + 1] == '>') {
                            tokens.push_back({ TokenType::RShift, ">>" });
                            ++i;
                        } else {
                            throw std::runtime_error(std::string("Unexpected character: '") + input[i] + "'");
                        }
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
            return tokens;
        }

        // Bytecode types
        enum class OpCode {
            PushConst,  // Push constants[operand] onto stack
            LoadVar,    // Push value of variable at symbols[operand]
            CallFunc,   // Call function at symbols[operand] with operand2 args from stack
            Add,
            Sub,
            Mul,
            Div,
            Pow,
            Neg,  // Negate top of stack
            BitAnd,
            BitOr,
            BitXor,
            BitNot,
            ShiftLeft,
            ShiftRight,
        };

        struct Instruction {
            OpCode op;
            std::size_t operand = 0;
            std::size_t operand2 = 0;
        };

        struct ByteCode {
            std::vector<Instruction> instructions;
            std::vector<Number> constants;
        };

        // Compiles tokens into bytecode by recursive descent
        class Compiler {
        public:
            explicit Compiler(std::vector<Token> const& tokens, std::vector<Symbol> const& symbols)
                : tokens_(tokens), symbols_(symbols) {}

            ByteCode compile() {
                expression();
                if (peek().type != TokenType::End) throw std::runtime_error("Unexpected token: '" + peek().text + "'");
                return { std::move(instructions_), std::move(constants_) };
            }

        private:
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

            void emit(OpCode op, std::size_t operand = 0, std::size_t operand2 = 0) {
                instructions_.push_back({ op, operand, operand2 });
            }

            std::size_t add_constant(Number value) {
                constants_.push_back(value);
                return constants_.size() - 1;
            }

            std::size_t find_symbol(std::string const& name) const {
                for (std::size_t i = 0; i < symbols_.size(); ++i) {
                    if (get_symbol_name(symbols_[i]) == name) return i;
                }
                throw std::runtime_error("Unknown symbol: '" + name + "'");
            }

            void expression() { bitwise_or(); }

            void bitwise_or() {
                bitwise_xor();
                while (peek().type == TokenType::Pipe) {
                    advance();
                    bitwise_xor();
                    emit(OpCode::BitOr);
                }
            }

            void bitwise_xor() {
                bitwise_and();
                while (peek().type == TokenType::Caret) {
                    advance();
                    bitwise_and();
                    emit(OpCode::BitXor);
                }
            }

            void bitwise_and() {
                shift();
                while (peek().type == TokenType::Ampersand) {
                    advance();
                    shift();
                    emit(OpCode::BitAnd);
                }
            }

            void shift() {
                additive();
                while (peek().type == TokenType::LShift || peek().type == TokenType::RShift) {
                    auto op = advance().type;
                    additive();
                    emit(op == TokenType::LShift ? OpCode::ShiftLeft : OpCode::ShiftRight);
                }
            }

            void additive() {
                term();
                while (peek().type == TokenType::Plus || peek().type == TokenType::Minus) {
                    auto op = advance().type;
                    term();
                    emit(op == TokenType::Plus ? OpCode::Add : OpCode::Sub);
                }
            }

            void term() {
                exponent();
                while (peek().type == TokenType::Star || peek().type == TokenType::Slash) {
                    auto op = advance().type;
                    exponent();
                    emit(op == TokenType::Star ? OpCode::Mul : OpCode::Div);
                }
            }

            void exponent() {
                unary();
                if (match(TokenType::StarStar)) {
                    exponent();
                    emit(OpCode::Pow);
                }
            }

            void unary() {
                switch (peek().type) {
                    case TokenType::Minus:
                        advance();
                        unary();
                        emit(OpCode::Neg);
                        return;
                    case TokenType::Plus:
                        advance();
                        unary();
                        return;
                    case TokenType::Tilde:
                        advance();
                        unary();
                        emit(OpCode::BitNot);
                        return;
                    default:
                        primary();
                        return;
                }
            }

            void primary() {
                if (peek().type == TokenType::NumberLiteral) {
                    emit(OpCode::PushConst, add_constant(advance().value));
                    return;
                }
                if (peek().type == TokenType::Identifier) {
                    auto name = advance().text;
                    if (peek().type == TokenType::LParen) {
                        advance();
                        std::size_t argc = 0;
                        if (peek().type != TokenType::RParen) {
                            expression();
                            ++argc;
                            while (match(TokenType::Comma)) {
                                expression();
                                ++argc;
                            }
                        }
                        expect(TokenType::RParen, "Expected ')'");
                        emit(OpCode::CallFunc, find_symbol(name), argc);
                        return;
                    }
                    emit(OpCode::LoadVar, find_symbol(name));
                    return;
                }
                if (peek().type == TokenType::LParen) {
                    advance();
                    expression();
                    expect(TokenType::RParen, "Expected ')'");
                    return;
                }
                throw std::runtime_error("Unexpected token: '" + peek().text + "'");
            }

            std::vector<Token> const& tokens_;
            std::vector<Symbol> const& symbols_;
            std::size_t pos_ = 0;
            std::vector<Instruction> instructions_;
            std::vector<Number> constants_;
        };

        // Execute bytecode against a symbol table using a stack-based VM
        inline Number execute(ByteCode const& code, std::vector<Symbol> const& symbols) {
            std::vector<Number> stack;
            stack.reserve(16);

            for (auto const& instr : code.instructions) {
                switch (instr.op) {
                    case OpCode::PushConst:
                        stack.push_back(code.constants[instr.operand]);
                        break;
                    case OpCode::LoadVar: {
                        auto const& sym = symbols[instr.operand];
                        if (auto* var = std::get_if<Variable>(&sym)) {
                            stack.push_back(var->get_value());
                        } else {
                            throw std::runtime_error("Symbol '" + get_symbol_name(sym) + "' is a function");
                        }
                        break;
                    }
                    case OpCode::CallFunc: {
                        auto const& sym = symbols[instr.operand];
                        auto argc = instr.operand2;
                        std::vector<Number> args(stack.end() - static_cast<std::ptrdiff_t>(argc), stack.end());
                        stack.resize(stack.size() - argc);
                        auto result = std::visit(
                            [&](auto const& s) -> Number {
                                using T = std::decay_t<decltype(s)>;
                                if constexpr (std::is_same_v<T, Variable>) {
                                    throw std::runtime_error("Symbol '" + s.get_name() + "' is not callable");
                                } else {
                                    return s.call(args);
                                }
                            },
                            sym);
                        stack.push_back(result);
                        break;
                    }
                    case OpCode::Add: {
                        auto b = stack.back();
                        stack.pop_back();
                        stack.back() += b;
                        break;
                    }
                    case OpCode::Sub: {
                        auto b = stack.back();
                        stack.pop_back();
                        stack.back() -= b;
                        break;
                    }
                    case OpCode::Mul: {
                        auto b = stack.back();
                        stack.pop_back();
                        stack.back() *= b;
                        break;
                    }
                    case OpCode::Div: {
                        auto b = stack.back();
                        stack.pop_back();
                        stack.back() /= b;
                        break;
                    }
                    case OpCode::Pow: {
                        auto b = stack.back();
                        stack.pop_back();
                        stack.back() = std::pow(stack.back(), b);
                        break;
                    }
                    case OpCode::Neg:
                        stack.back() = -stack.back();
                        break;
                    case OpCode::BitAnd: {
                        auto b = static_cast<Int>(stack.back());
                        stack.pop_back();
                        stack.back() = static_cast<Number>(static_cast<Int>(stack.back()) & b);
                        break;
                    }
                    case OpCode::BitOr: {
                        auto b = static_cast<Int>(stack.back());
                        stack.pop_back();
                        stack.back() = static_cast<Number>(static_cast<Int>(stack.back()) | b);
                        break;
                    }
                    case OpCode::BitXor: {
                        auto b = static_cast<Int>(stack.back());
                        stack.pop_back();
                        stack.back() = static_cast<Number>(static_cast<Int>(stack.back()) ^ b);
                        break;
                    }
                    case OpCode::BitNot:
                        stack.back() = static_cast<Number>(~static_cast<Int>(stack.back()));
                        break;
                    case OpCode::ShiftLeft: {
                        auto b = static_cast<Int>(stack.back());
                        stack.pop_back();
                        stack.back() = static_cast<Number>(static_cast<Int>(stack.back()) << b);
                        break;
                    }
                    case OpCode::ShiftRight: {
                        auto b = static_cast<Int>(stack.back());
                        stack.pop_back();
                        stack.back() = static_cast<Number>(static_cast<Int>(stack.back()) >> b);
                        break;
                    }
                }
            }
            return stack.back();
        }
    }

    class Expression;

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
        // Deduces the number of function arguments from the type of the callable
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

        // Define a new function or update an existing one
        // Explicit number of function arguments, allows for `auto` parameters in lambdas
        template <std::size_t N>
        void define(std::string name, typename Function<N>::Callback function) {
            static_assert(N <= 5, "Functions with more than 5 arguments are not supported");
            for (auto& sym : symbols_) {
                if (auto* func = std::get_if<Function<N>>(&sym)) {
                    if (func->get_name() == name) {
                        func->set_function(std::move(function));
                        return;
                    }
                }
            }
            symbols_.push_back(Function<N>(std::move(name), std::move(function)));
        }

        // Undefine a symbol by name
        void undefine(std::string_view name) {
            std::ranges::remove_if(symbols_, [&](Symbol const& sym) { return get_symbol_name(sym) == name; });
        }

        // Evaluate an expression
        // To improve performance for repeated evaluations,
        // compile it first with `compile()` and then call
        // `evaluate()` on the resulting `Expression` object
        [[nodiscard]]
        Number evaluate(std::string_view input) const {
            using namespace detail;
            std::vector<Token> tokens = tokenize(input);
            Compiler compiler(tokens, symbols_);
            ByteCode bytecode = compiler.compile();
            return execute(bytecode, symbols_);
        }

        // Compile an expression for efficient repeated evaluation
        // This call does not execute the expression,
        // Call `evaluate()` on the resulting `Expression` object to execute it
        [[nodiscard]]
        Expression compile(std::string_view input) const;

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
        void define_builtins() {
            // Constants
            define("pi", std::numbers::pi);
            define("e", std::numbers::e);

            // Unary functions
            define<1>("sin", [](auto x) { return std::sin(x); });
            define<1>("cos", [](auto x) { return std::cos(x); });
            define<1>("tan", [](auto x) { return std::tan(x); });
            define<1>("asin", [](auto x) { return std::asin(x); });
            define<1>("acos", [](auto x) { return std::acos(x); });
            define<1>("atan", [](auto x) { return std::atan(x); });
            define<1>("sqrt", [](auto x) { return std::sqrt(x); });
            define<1>("cbrt", [](auto x) { return std::cbrt(x); });
            define<1>("abs", [](auto x) { return std::abs(x); });
            define<1>("ln", [](auto x) { return std::log(x); });
            define<1>("log2", [](auto x) { return std::log2(x); });
            define<1>("log10", [](auto x) { return std::log10(x); });
            define<1>("exp", [](auto x) { return std::exp(x); });
            define<1>("floor", [](auto x) { return std::floor(x); });
            define<1>("ceil", [](auto x) { return std::ceil(x); });
            define<1>("round", [](auto x) { return std::round(x); });
            define<1>("deg2rad", [](auto x) { return x * std::numbers::pi / 180; });
            define<1>("rad2deg", [](auto x) { return x * 180 / std::numbers::pi; });

            // Binary functions
            define<2>("pow", [](auto a, auto b) { return std::pow(a, b); });
            define<2>("atan2", [](auto y, auto x) { return std::atan2(y, x); });
            define<2>("min", [](auto a, auto b) { return std::min(a, b); });
            define<2>("max", [](auto a, auto b) { return std::max(a, b); });
            define<2>("log", [](auto base, auto x) { return std::log(x) / std::log(base); });
        }

        std::vector<Symbol> symbols_;
    };

    class Expression {
    public:
        explicit Expression(Context const& context, std::string_view input) : context_(context) {
            auto tokens = detail::tokenize(input);
            detail::Compiler compiler(tokens, context.get_symbols());
            bytecode_ = compiler.compile();
        }

        [[nodiscard]]
        Number evaluate() const {
            return detail::execute(bytecode_, context_.get_symbols());
        }

    private:
        Context const& context_;
        detail::ByteCode bytecode_;
    };

    inline Expression Context::compile(std::string_view input) const {
        return Expression(*this, input);
    }
}