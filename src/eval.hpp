#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <memory>
#include "rivet/ast.hpp"

namespace rivet {

// ---- Forward decl so Value can hold arrays and arrays can hold Values.
struct Array;

// Runtime value: number | bool | string | array
using Value = std::variant<double, bool, std::string, std::shared_ptr<Array>>;

// Now that Value is defined, we can define Array using Value.
struct Array { std::vector<Value> items; };

// --- helpers ---
inline bool is_number(const Value& v){ return std::holds_alternative<double>(v); }
inline bool is_bool  (const Value& v){ return std::holds_alternative<bool>(v); }
inline bool is_string(const Value& v){ return std::holds_alternative<std::string>(v); }
inline bool is_array (const Value& v){ return std::holds_alternative<std::shared_ptr<Array>>(v); }

inline double as_number(const Value& v){ return std::get<double>(v); }
inline bool as_bool(const Value& v){ return std::get<bool>(v); }
inline const std::string& as_string(const Value& v){ return std::get<std::string>(v); }
inline std::shared_ptr<Array> as_array(const Value& v){ return std::get<std::shared_ptr<Array>>(v); }

inline bool truthy(const Value& v) {
  if (is_bool(v))   return as_bool(v);
  if (is_number(v)) return as_number(v) != 0.0;
  if (is_string(v)) return !as_string(v).empty();
  if (is_array(v))  return !as_array(v)->items.empty();
  return false;
}

struct VarCell { Value val{}; bool mut{}; }; // mut=false => let, true => var

class Env {
public:
  void push();
  void pop();

  void define_let(const std::string& name, Value v);
  void define_var(const std::string& name, Value v);
  void assign(const std::string& name, Value v);
  bool  get(const std::string& name, Value& out) const;

  void define_fn(const FnDecl* fn);
  const FnDecl* get_fn(const std::string& name) const;

private:
  std::vector<std::unordered_map<std::string, VarCell>> scopes;
  std::unordered_map<std::string, const FnDecl*> fns;
};

// Evaluate a single expression
Value eval_expr(const Expr& e, const Env& env);

// Execute a statement; may return a value (expr stmt or return)
std::optional<Value> exec_stmt(const Stmt& s, Env& env,
                               bool* returned = nullptr,
                               Value* ret_val = nullptr);

// Execute whole program
std::optional<Value> exec_program(const Program& p, Env& env);

} // namespace rivet