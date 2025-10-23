#pragma once
#include <string>
#include "lexer.hpp"
#include "rivet/ast.hpp"

namespace rivet {

class Parser {
public:
  explicit Parser(std::string source, std::string filename = "<stdin>");
  Program parse_program();
  StmtPtr parse_one_stmt();

private:
  // Statements
  StmtPtr statement();
  StmtPtr let_stmt();
  StmtPtr var_stmt();
  StmtPtr assign_or_expr_stmt();
  StmtPtr block_stmt();
  StmtPtr if_stmt();
  StmtPtr while_stmt();
  StmtPtr for_stmt();       // either for-in or C-style
  StmtPtr print_stmt();
  StmtPtr fn_decl();
  StmtPtr return_stmt();

  // Expressions (precedence)
  // expr       := or
  // or         := and ( "||" and )*
  // and        := equality ( "&&" equality )*
  // equality   := comparison ( (== | !=) comparison )*
  // comparison := term ( (< | <= | > | >= ) term )*
  // term       := factor ( ("+"|"-") factor )*
  // factor     := unary  ( ("*"|"/") unary )*
  // unary      := ("-"|"!") unary | call
  // call       := primary ( "(" args? ")" )*
  // primary    := NUMBER | STRING | true | false | IDENT | array | "(" expr ")"
  // array      := "[" (expr ("," expr)*)? "]"
  ExprPtr expression();
  ExprPtr or_expr();
  ExprPtr and_expr();
  ExprPtr equality();
  ExprPtr comparison();
  ExprPtr term();
  ExprPtr factor();
  ExprPtr unary();
  ExprPtr call();
  ExprPtr primary();
  ExprPtr array_lit();
  std::vector<ExprPtr> arg_list();

  // Token utils
  const Token& advance();
  const Token& peek() const { return current; }
  bool check(TokenKind k) const { return current.kind == k; }
  bool match(TokenKind k);
  const Token& expect(TokenKind k, const char* msg);

private:
  Lexer      lex;
  Token      current;
  std::string filename;
};

} // namespace rivet