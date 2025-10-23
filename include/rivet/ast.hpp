#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "rivet/token.hpp"

namespace rivet {

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

// Supported literal runtime value (for now): double
using Number = double;

struct Literal {
  Number value;
};

struct Grouping {
  ExprPtr inner;
};

enum class UnaryOp { Negate /* '-' */ };
struct Unary {
  UnaryOp op;
  ExprPtr right;
};

enum class BinaryOp { Add, Sub, Mul, Div };
struct Binary {
  ExprPtr left;
  BinaryOp op;
  ExprPtr right;
};

struct Expr {
  // A simple sum type via std::variant
  std::variant<Literal, Grouping, Unary, Binary> node;

  // Helpers to build nodes
  static ExprPtr make_literal(Number v) {
    return std::make_unique<Expr>(Expr{Literal{v}});
  }
  static ExprPtr make_grouping(ExprPtr e) {
    return std::make_unique<Expr>(Expr{Grouping{std::move(e)}});
  }
  static ExprPtr make_unary(UnaryOp op, ExprPtr r) {
    return std::make_unique<Expr>(Expr{Unary{op, std::move(r)}});
  }
  static ExprPtr make_binary(ExprPtr l, BinaryOp op, ExprPtr r) {
    return std::make_unique<Expr>(Expr{Binary{std::move(l), op, std::move(r)}});
  }
};

} // namespace rivet