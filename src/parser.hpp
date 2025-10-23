#pragma once
#include <string>
#include <optional>
#include "lexer.hpp"
#include "rivet/ast.hpp"

namespace rivet {

class Parser {
public:
  explicit Parser(std::string source, std::string filename = "<stdin>");

  // Parse a single expression till End
  // Throws std::runtime_error on parse errors
  ExprPtr parse_expression();

private:
  // Grammar (recursive descent with precedence):
  // expr     := term (("+"|"-") term)*
  // term     := factor (("*"|"/") factor)*
  // factor   := ("-" factor) | primary
  // primary  := NUMBER | "(" expr ")"
  ExprPtr expression();
  ExprPtr term();
  ExprPtr factor();
  ExprPtr primary();

  // Token utilities
  const Token& advance();
  const Token& peek() const { return current; }
  bool check(TokenKind k) const { return current.kind == k; }
  bool match(TokenKind k);
  const Token& expect(TokenKind k, const char* msg);

private:
  Lexer      lex;
  Token      current;   // current lookahead
  std::string filename;
};

} // namespace rivet