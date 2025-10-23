#include "eval.hpp"
#include <stdexcept>
#include <type_traits>   // <-- add this

namespace rivet {

static double eval_node(const Expr& e);

// helper for clean static_assert in dependent context
template<class> inline constexpr bool always_false_v = false;

static double eval_literal(const Literal& n) {
  return n.value;
}

static double eval_grouping(const Grouping& g) {
  return eval_node(*g.inner);
}

static double eval_unary(const Unary& u) {
  double r = eval_node(*u.right);
  switch (u.op) {
    case UnaryOp::Negate: return -r;
  }
  throw std::runtime_error("eval: unknown unary op");
}

static double eval_binary(const Binary& b) {
  double l = eval_node(*b.left);
  double r = eval_node(*b.right);
  switch (b.op) {
    case BinaryOp::Add: return l + r;
    case BinaryOp::Sub: return l - r;
    case BinaryOp::Mul: return l * r;
    case BinaryOp::Div:
      if (r == 0.0) throw std::runtime_error("runtime error: division by zero");
      return l / r;
  }
  throw std::runtime_error("eval: unknown binary op");
}

static double eval_node(const Expr& e) {
  return std::visit([](auto const& node) -> double {
    using T = std::decay_t<decltype(node)>;
    if constexpr (std::is_same_v<T, Literal>)   return eval_literal(node);
    else if constexpr (std::is_same_v<T, Grouping>)  return eval_grouping(node);
    else if constexpr (std::is_same_v<T, Unary>)     return eval_unary(node);
    else if constexpr (std::is_same_v<T, Binary>)    return eval_binary(node);
    else {
      static_assert(always_false_v<T>, "Unhandled Expr node");
      return 0.0; // keeps some compilers satisfied; never instantiated
    }
  }, e.node);
}

double eval_expr(const Expr& e) {
  return eval_node(e);
}

} // namespace rivet