#include "eval.hpp"
#include <stdexcept>
#include <type_traits>
#include <iostream>
#include <sstream>
#include <string>

namespace rivet {

template<class> inline constexpr bool always_false_v = false;

// ========== Env ==========
void Env::push() { scopes.emplace_back(); }
void Env::pop()  { if (!scopes.empty()) scopes.pop_back(); }

void Env::define_let(const std::string& name, Value v) {
  if (scopes.empty()) push();
  scopes.back()[name] = VarCell{std::move(v), false};
}
void Env::define_var(const std::string& name, Value v) {
  if (scopes.empty()) push();
  scopes.back()[name] = VarCell{std::move(v), true};
}
void Env::assign(const std::string& name, Value v) {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto f = it->find(name);
    if (f != it->end()) {
      if (!f->second.mut)
        throw std::runtime_error("runtime error: cannot assign to immutable 'let " + name + "'");
      f->second.val = std::move(v);
      return;
    }
  }
  throw std::runtime_error("runtime error: assignment to undefined variable '" + name + "'");
}
bool Env::get(const std::string& name, Value& out) const {
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    auto f = it->find(name);
    if (f != it->end()) { out = f->second.val; return true; }
  }
  return false;
}
void Env::define_fn(const FnDecl* fn) { fns[fn->name] = fn; }
const FnDecl* Env::get_fn(const std::string& name) const {
  auto it = fns.find(name);
  return it == fns.end() ? nullptr : it->second;
}

// ========== helpers ==========
static std::string to_string_value(const Value& v) {
  if (is_number(v)) { std::ostringstream os; os << as_number(v); return os.str(); }
  if (is_bool(v))   return as_bool(v) ? "true" : "false";
  if (is_string(v)) return as_string(v);
  const auto& arr = *as_array(v);
  std::string s = "[";
  for (size_t i = 0; i < arr.items.size(); ++i) {
    if (i) s += ", ";
    s += to_string_value(arr.items[i]);
  }
  s += "]";
  return s;
}

static bool equal_values(const Value& a, const Value& b) {
  if (a.index() != b.index()) return false;
  if (is_number(a)) return as_number(a) == as_number(b);
  if (is_bool(a))   return as_bool(a)   == as_bool(b);
  if (is_string(a)) return as_string(a) == as_string(b);
  auto A = as_array(a), B = as_array(b);
  if (A->items.size() != B->items.size()) return false;
  for (size_t i = 0; i < A->items.size(); ++i)
    if (!equal_values(A->items[i], B->items[i])) return false;
  return true;
}

static Value eval_node(const Expr& e, const Env& env);
static Value eval_call(const Call& c, Env& env);

// ========== expr ==========
static Value eval_number(const NumberLit& n){ return n.value; }
static Value eval_bool  (const BoolLit& b){ return b.value; }
static Value eval_string(const StringLit& s){ return s.value; }
static Value eval_array (const ArrayLit& a, const Env& env){
  auto out = std::make_shared<Array>();
  out->items.reserve(a.elems.size());
  for (auto& e : a.elems) out->items.push_back(eval_node(*e, env));
  return out;
}
static Value eval_group (const Grouping& g, const Env& env){ return eval_node(*g.inner, env); }

static Value eval_unary(const Unary& u, const Env& env){
  Value r = eval_node(*u.right, env);
  switch (u.op) {
    case UnaryOp::Negate:
      if (!is_number(r)) throw std::runtime_error("type error: unary '-' expects number");
      return -as_number(r);
    case UnaryOp::Not:
      return !truthy(r);
  }
  throw std::runtime_error("eval: unknown unary op");
}

static Value eval_binary(const Binary& b, const Env& env){
  if (b.op == BinaryOp::LOr)  { Value l = eval_node(*b.left, env); if (truthy(l)) return true;  Value r = eval_node(*b.right, env); return truthy(r); }
  if (b.op == BinaryOp::LAnd) { Value l = eval_node(*b.left, env); if (!truthy(l)) return false; Value r = eval_node(*b.right, env); return truthy(r); }

  Value l = eval_node(*b.left, env);
  Value r = eval_node(*b.right, env);
  switch (b.op) {
    case BinaryOp::Add:
      if (is_number(l) && is_number(r)) return as_number(l) + as_number(r);
      if (is_string(l) || is_string(r)) return to_string_value(l) + to_string_value(r);
      throw std::runtime_error("type error: '+' expects number+number or string (+ anything)");
    case BinaryOp::Sub:
      if (is_number(l) && is_number(r)) return as_number(l) - as_number(r);
      throw std::runtime_error("type error: '-' expects numbers");
    case BinaryOp::Mul:
      if (is_number(l) && is_number(r)) return as_number(l) * as_number(r);
      throw std::runtime_error("type error: '*' expects numbers");
    case BinaryOp::Div:
      if (is_number(l) && is_number(r)) {
        if (as_number(r) == 0.0) throw std::runtime_error("runtime error: division by zero");
        return as_number(l) / as_number(r);
      }
      throw std::runtime_error("type error: '/' expects numbers");
    case BinaryOp::Eq:  return equal_values(l, r);
    case BinaryOp::Ne:  return !equal_values(l, r);
    case BinaryOp::Lt:
      if (is_number(l)&&is_number(r)) return as_number(l) <  as_number(r);
      if (is_string(l)&&is_string(r)) return as_string(l) <  as_string(r);
      throw std::runtime_error("type error: '<' expects number/number or string/string");
    case BinaryOp::Le:
      if (is_number(l)&&is_number(r)) return as_number(l) <= as_number(r);
      if (is_string(l)&&is_string(r)) return as_string(l) <= as_string(r);
      throw std::runtime_error("type error: '<=' expects number/number or string/string");
    case BinaryOp::Gt:
      if (is_number(l)&&is_number(r)) return as_number(l) >  as_number(r);
      if (is_string(l)&&is_string(r)) return as_string(l) >  as_string(r);
      throw std::runtime_error("type error: '>' expects number/number or string/string");
    case BinaryOp::Ge:
      if (is_number(l)&&is_number(r)) return as_number(l) >= as_number(r);
      if (is_string(l)&&is_string(r)) return as_string(l) >= as_string(r);
      throw std::runtime_error("type error: '>=' expects number/number or string/string");
    default: break;
  }
  throw std::runtime_error("eval: unknown binary op");
}

static Value eval_variable(const Variable& v, const Env& env){
  Value out;
  if (!env.get(v.name, out))
    throw std::runtime_error("runtime error: undefined variable '" + v.name + "'");
  return out;
}

static Value eval_node(const Expr& e, const Env& env_ro){
  Env& env = const_cast<Env&>(env_ro);
  return std::visit([&](auto const& node) -> Value {
    using T = std::decay_t<decltype(node)>;
    if constexpr (std::is_same_v<T, NumberLit>) return eval_number(node);
    else if constexpr (std::is_same_v<T, BoolLit>) return eval_bool(node);
    else if constexpr (std::is_same_v<T, StringLit>) return eval_string(node);
    else if constexpr (std::is_same_v<T, ArrayLit>) return eval_array(node, env);
    else if constexpr (std::is_same_v<T, Grouping>)  return eval_group(node, env);
    else if constexpr (std::is_same_v<T, Unary>)     return eval_unary(node, env);
    else if constexpr (std::is_same_v<T, Binary>)    return eval_binary(node, env);
    else if constexpr (std::is_same_v<T, Variable>)  return eval_variable(node, env);
    else if constexpr (std::is_same_v<T, Call>)      return eval_call(node, env);
    else { static_assert(always_false_v<T>, "Unhandled Expr node"); return {}; }
  }, e.node);
}

Value eval_expr(const Expr& e, const Env& env) { return eval_node(e, env); }

// ========== Stmts ==========
std::optional<Value> exec_stmt(const Stmt& s, Env& env, bool* returned, Value* ret_val){
  auto mark_return = [&](Value v){ if (returned) *returned = true; if (ret_val) *ret_val = std::move(v); };

  return std::visit([&](auto const& node) -> std::optional<Value> {
    using T = std::decay_t<decltype(node)>;

    if constexpr (std::is_same_v<T, Let>) {
      Value v = eval_node(*node.init, env);
      env.define_let(node.name, std::move(v));
      return std::nullopt;

    } else if constexpr (std::is_same_v<T, Var>) {
      Value v = eval_node(*node.init, env);
      env.define_var(node.name, std::move(v));
      return std::nullopt;

    } else if constexpr (std::is_same_v<T, Assign>) {
      Value v = eval_node(*node.value, env);
      env.assign(node.name, std::move(v));
      return std::nullopt;

    } else if constexpr (std::is_same_v<T, ExprStmt>) {
      return eval_node(*node.expr, env);

    } else if constexpr (std::is_same_v<T, Print>) {
      Value v = eval_node(*node.expr, env);
      std::cout << to_string_value(v) << "\n";
      return std::nullopt;

    } else if constexpr (std::is_same_v<T, Block>) {
      env.push();
      std::optional<Value> last;
      for (auto const& st : node.stmts) {
        bool ret = false; Value rv{};
        last = exec_stmt(*st, env, &ret, &rv);
        if (ret) { env.pop(); mark_return(std::move(rv)); return std::nullopt; }
      }
      env.pop();
      return last;

    } else if constexpr (std::is_same_v<T, If>) {
      Value c = eval_node(*node.cond, env);
      return exec_stmt(truthy(c) ? *node.then_br : *node.else_br, env, returned, ret_val);

    } else if constexpr (std::is_same_v<T, While>) {
      std::optional<Value> last;
      while (truthy(eval_node(*node.cond, env))) {
        bool ret = false; Value rv{};
        last = exec_stmt(*node.body, env, &ret, &rv);
        if (ret) { mark_return(std::move(rv)); return std::nullopt; }
      }
      return last;

    } else if constexpr (std::is_same_v<T, ForC>) {
      env.push();
      if (node.init) (void)exec_stmt(*node.init, env);
      std::optional<Value> last;
      while (!node.cond || truthy(eval_node(*node.cond, env))) {
        bool ret = false; Value rv{};
        last = exec_stmt(*node.body, env, &ret, &rv);
        if (ret) { env.pop(); mark_return(std::move(rv)); return std::nullopt; }
        if (node.step) {
          bool sret = false; Value srv{};
          (void)exec_stmt(*node.step, env, &sret, &srv);
          if (sret) { env.pop(); mark_return(std::move(srv)); return std::nullopt; }
        }
      }
      env.pop();
      return last;

    } else if constexpr (std::is_same_v<T, ForIn>) {
      Value iter = eval_node(*node.iterable, env);
      if (is_array(iter)) {
        auto arr = as_array(iter);
        for (auto& v : arr->items) {
          env.push();
          env.define_var(node.var, v);
          bool ret = false; Value rv{};
          (void)exec_stmt(*node.body, env, &ret, &rv);
          env.pop();
          if (ret) { mark_return(std::move(rv)); return std::nullopt; }
        }
        return std::nullopt;
      } else if (is_string(iter)) {
        const auto& s = as_string(iter);
        for (char ch : s) {
          env.push();
          env.define_var(node.var, std::string(1, ch));
          bool ret = false; Value rv{};
          (void)exec_stmt(*node.body, env, &ret, &rv);
          env.pop();
          if (ret) { mark_return(std::move(rv)); return std::nullopt; }
        }
        return std::nullopt;
      }
      throw std::runtime_error("type error: for-in expects array or string");

    } else if constexpr (std::is_same_v<T, FnDecl>) {
      env.define_fn(&node);
      return std::nullopt;

    } else if constexpr (std::is_same_v<T, Return>) {
      Value v = eval_node(*node.value, env);
      mark_return(std::move(v));
      return std::nullopt;

    } else {
      static_assert(always_false_v<T>, "Unhandled Stmt node");
      return std::nullopt;
    }
  }, s.node);
}

std::optional<Value> exec_program(const Program& p, Env& env) {
  std::optional<Value> last;
  for (auto const& s : p) {
    bool ret = false; Value rv{};
    last = exec_stmt(*s, env, &ret, &rv);
    if (ret) return rv;
  }
  return last;
}

// ========== Calls ==========
static Value eval_call(const Call& c, Env& env) {
  const FnDecl* fn = env.get_fn(c.callee);
  if (!fn) throw std::runtime_error("runtime error: undefined function '" + c.callee + "'");
  if (c.args.size() != fn->params.size())
    throw std::runtime_error("runtime error: function '" + c.callee + "' arity mismatch");

  env.push();
  for (size_t i = 0; i < c.args.size(); ++i) {
    Value v = eval_node(*c.args[i], env);
    env.define_var(fn->params[i], std::move(v));
  }
  bool ret = false; Value rv{};
  exec_stmt(*fn->body, env, &ret, &rv);
  env.pop();
  if (!ret) return 0.0;
  return rv;
}

}