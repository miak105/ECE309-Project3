// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    *error = "calc_evaluate not implemented yet";  

    using Kind = CalcToken::Kind;

    // One stack holds numbers, the other operators.
    std::stack<double> values;
    std::stack<Kind> ops;

    // Multiplication and division happen first.
    auto precedence = [](Kind op) -> int {
        if (op == Kind::Times || op == Kind::Divide)
            return 2;
        if (op == Kind::Plus || op == Kind::Minus)
            return 1;
        return 0;
    };

    // Apply the operator on top of the stack.
    auto apply = [&]() -> bool {
        if (ops.empty() || values.size() < 2) {
            *error = "invalid expression";
            return false;
        }

        Kind op = ops.top();
        ops.pop();

        double right = values.top();
        values.pop();

        double left = values.top();
        values.pop();

        double result = 0;

        switch (op) {
            case Kind::Plus:
                result = left + right;
                break;
            case Kind::Minus:
                result = left - right;
                break;
            case Kind::Times:
                result = left * right;
                break;
            case Kind::Divide:
                if (right == 0) {
                    *error = "division by zero";
                    return false;
                }
                result = left / right;
                break;
            default:
                *error = "invalid operator";
                return false;
        }

        values.push(result);
        return true;
    };

    // Process each token from left to right.
    for (const auto& token : tokens) {
        switch (token.kind) {
            case Kind::Number:
                values.push(token.value);
                break;

            case Kind::LParen:
                ops.push(token.kind);
                break;

            case Kind::RParen:
                while (!ops.empty() &&
                       ops.top() != Kind::LParen) {
                    if (!apply()) return false;
                }

                if (ops.empty()) {
                    *error = "unmatched parenthesis";
                    return false;
                }

                ops.pop();
                break;

            case Kind::Plus:
            case Kind::Minus:
            case Kind::Times:
            case Kind::Divide:
                while (!ops.empty() &&
                       ops.top() != Kind::LParen &&
                       precedence(ops.top()) >=
                           precedence(token.kind)) {
                    if (!apply()) return false;
                }

                ops.push(token.kind);
                break;
        }
    }

    // Apply any operators left on the stack.
    while (!ops.empty()) {
        if (ops.top() == Kind::LParen) {
            *error = "unmatched parenthesis";
            return false;
        }

        if (!apply()) return false;
    }

    if (values.size() != 1) {
        *error = "invalid expression";
        return false;
    }

    *out = values.top();
    return true;
}
