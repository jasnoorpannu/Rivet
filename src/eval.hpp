#pragma once
#include "rivet/ast.hpp"

namespace rivet {

// Evaluate a numeric expression tree and return its double result.
// Throws std::runtime_error on errors (e.g., divide by zero).
double eval_expr(const Expr& e);

} // namespace rivet