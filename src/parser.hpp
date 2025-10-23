#pragma once
#include <string>
#include <vector>
#include "lexer.hpp"
#include "rivet/ast.hpp"

namespace rivet {

class Parser {
public:
  explicit Parser(std::string source, std::string filename = "<stdin>");

  Program parse_program();        // parse whole file (0+ statements)
  StmtPtr parse_one_stmt();       // parse a single statement (REPL)

private:
  // ---- Statements ----
  StmtPtr statement();
  StmtPtr let_stmt();                 // let x = expr ;
  StmtPtr var_stmt();                 // var x = expr ;
  StmtPtr assign_or_expr_stmt();      // IDENT '=' expr ';' | expr ';'
  StmtPtr block_stmt();
  StmtPtr if_stmt();
  StmtPtr while_stmt();
  StmtPtr for_stmt();                 // 'for' ...   (for-in or C-style)
  StmtPtr print_stmt();
  StmtPtr fn_decl();
  StmtPtr return_stmt();

  // ---- Helpers for C-style for(...) so we don't consume its separators ----
  StmtPtr let_decl_no_semi();         // let x = expr         (NO trailing ';')
  StmtPtr var_decl_no_semi();         // var x = expr         (NO trailing ';')
  StmtPtr assign_or_expr_no_semi();   // IDENT '=' expr | expr (NO trailing ';')

  // ---- Expressions (precedence climbing) ----
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

  // ---- Token utilities ----
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