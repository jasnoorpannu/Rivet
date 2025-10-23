#pragma once
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "rivet/token.hpp"

namespace rivet {

// ============== Expressions ==============
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct NumberLit { double value; };
struct BoolLit   { bool   value; };
struct StringLit { std::string value; };
struct ArrayLit  { std::vector<ExprPtr> elems; };
struct Grouping  { ExprPtr inner; };

enum class UnaryOp { Negate, Not };
struct Unary { UnaryOp op; ExprPtr right; };

enum class BinaryOp { Add, Sub, Mul, Div, Eq, Ne, Lt, Le, Gt, Ge, LAnd, LOr };
struct Binary { ExprPtr left; BinaryOp op; ExprPtr right; };

struct Variable { std::string name; };

struct Call {
  std::string callee;
  std::vector<ExprPtr> args;
};

struct Expr {
  std::variant<NumberLit, BoolLit, StringLit, ArrayLit, Grouping, Unary, Binary, Variable, Call> node;

  static ExprPtr make_number(double v){ return std::make_unique<Expr>(Expr{NumberLit{v}}); }
  static ExprPtr make_bool(bool v){ return std::make_unique<Expr>(Expr{BoolLit{v}}); }
  static ExprPtr make_string(std::string v){ return std::make_unique<Expr>(Expr{StringLit{std::move(v)}}); }
  static ExprPtr make_array(std::vector<ExprPtr> es){ return std::make_unique<Expr>(Expr{ArrayLit{std::move(es)}}); }
  static ExprPtr make_grouping(ExprPtr e){ return std::make_unique<Expr>(Expr{Grouping{std::move(e)}}); }
  static ExprPtr make_unary(UnaryOp op, ExprPtr r){ return std::make_unique<Expr>(Expr{Unary{op, std::move(r)}}); }
  static ExprPtr make_binary(ExprPtr l, BinaryOp op, ExprPtr r){ return std::make_unique<Expr>(Expr{Binary{std::move(l), op, std::move(r)}}); }
  static ExprPtr make_variable(std::string n){ return std::make_unique<Expr>(Expr{Variable{std::move(n)}}); }
  static ExprPtr make_call(std::string n, std::vector<ExprPtr> as){ return std::make_unique<Expr>(Expr{Call{std::move(n), std::move(as)}}); }
};

// ============== Statements ==============
struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct Let     { std::string name; ExprPtr init; };
struct Var     { std::string name; ExprPtr init; };
struct Assign  { std::string name; ExprPtr value; };
struct ExprStmt{ ExprPtr expr; };
struct Block   { std::vector<StmtPtr> stmts; };
struct If      { ExprPtr cond; StmtPtr then_br; StmtPtr else_br; };
struct While   { ExprPtr cond; StmtPtr body; };
struct Print   { ExprPtr expr; };
struct FnDecl  { std::string name; std::vector<std::string> params; StmtPtr body; };
struct Return  { ExprPtr value; };

// for-in: for ident in expr { ... }
struct ForIn   { std::string var; ExprPtr iterable; StmtPtr body; };

// C-style for: for (init; cond; step) body
// NOTE: step is a STATEMENT now (e.g., assignment or call), not an expression.
struct ForC    { StmtPtr init; ExprPtr cond; StmtPtr step; StmtPtr body; };

struct Stmt {
  std::variant<Let, Var, Assign, ExprStmt, Block, If, While, Print, FnDecl, Return, ForIn, ForC> node;

  static StmtPtr make_let(std::string n, ExprPtr e){ return std::make_unique<Stmt>(Stmt{Let{std::move(n), std::move(e)}}); }
  static StmtPtr make_var(std::string n, ExprPtr e){ return std::make_unique<Stmt>(Stmt{Var{std::move(n), std::move(e)}}); }
  static StmtPtr make_assign(std::string n, ExprPtr e){ return std::make_unique<Stmt>(Stmt{Assign{std::move(n), std::move(e)}}); }
  static StmtPtr make_expr(ExprPtr e){ return std::make_unique<Stmt>(Stmt{ExprStmt{std::move(e)}}); }
  static StmtPtr make_block(std::vector<StmtPtr> ss){ return std::make_unique<Stmt>(Stmt{Block{std::move(ss)}}); }
  static StmtPtr make_if(ExprPtr c, StmtPtr t, StmtPtr e){ return std::make_unique<Stmt>(Stmt{If{std::move(c), std::move(t), std::move(e)}}); }
  static StmtPtr make_while(ExprPtr c, StmtPtr b){ return std::make_unique<Stmt>(Stmt{While{std::move(c), std::move(b)}}); }
  static StmtPtr make_print(ExprPtr e){ return std::make_unique<Stmt>(Stmt{Print{std::move(e)}}); }
  static StmtPtr make_fn(std::string n, std::vector<std::string> ps, StmtPtr b){ return std::make_unique<Stmt>(Stmt{FnDecl{std::move(n), std::move(ps), std::move(b)}}); }
  static StmtPtr make_return(ExprPtr v){ return std::make_unique<Stmt>(Stmt{Return{std::move(v)}}); }
  static StmtPtr make_for_in(std::string v, ExprPtr it, StmtPtr b){ return std::make_unique<Stmt>(Stmt{ForIn{std::move(v), std::move(it), std::move(b)}}); }
  static StmtPtr make_for_c(StmtPtr i, ExprPtr c, StmtPtr s, StmtPtr b){ return std::make_unique<Stmt>(Stmt{ForC{std::move(i), std::move(c), std::move(s), std::move(b)}}); }
};

using Program = std::vector<StmtPtr>;

} // namespace rivet