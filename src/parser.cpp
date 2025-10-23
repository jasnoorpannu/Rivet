#include "parser.hpp"
#include <stdexcept>
#include <sstream>
#include <cstdlib>

namespace rivet {

static std::string pos_str(const std::string& file, const Token& t) {
  std::ostringstream oss;
  oss << file << ":" << t.pos.line << ":" << t.pos.col << ": ";
  return oss.str();
}

Parser::Parser(std::string source, std::string filename_)
  : lex(std::move(source), filename_), filename(std::move(filename_))
{
  // Prime the current token
  current = lex.next();
  if (current.kind == TokenKind::Error) {
    throw std::runtime_error(pos_str(filename, current) + "lex error: " + current.lexeme);
  }
}

const Token& Parser::advance() {
  current = lex.next();
  if (current.kind == TokenKind::Error) {
    throw std::runtime_error(pos_str(filename, current) + "lex error: " + current.lexeme);
  }
  return current;
}

bool Parser::match(TokenKind k) {
  if (check(k)) { advance(); return true; }
  return false;
}

const Token& Parser::expect(TokenKind k, const char* msg) {
  if (!check(k)) {
    throw std::runtime_error(pos_str(filename, current) + std::string("parse error: expected ") + msg);
  }
  return advance();
}

ExprPtr Parser::parse_expression() {
  auto expr = expression();
  // Accept optional semicolon, then End
  if (check(TokenKind::Semicolon)) advance();
  if (!check(TokenKind::End)) {
    throw std::runtime_error(pos_str(filename, current) + "parse error: expected end of input");
  }
  return expr;
}

// expr := term (("+"|"-") term)*
ExprPtr Parser::expression() {
  auto left = term();
  while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
    Token op = current;
    advance();
    auto right = term();
    auto bop = (op.kind == TokenKind::Plus) ? BinaryOp::Add : BinaryOp::Sub;
    left = Expr::make_binary(std::move(left), bop, std::move(right));
  }
  return left;
}

// term := factor (("*"|"/") factor)*
ExprPtr Parser::term() {
  auto left = factor();
  while (check(TokenKind::Star) || check(TokenKind::Slash)) {
    Token op = current;
    advance();
    auto right = factor();
    auto bop = (op.kind == TokenKind::Star) ? BinaryOp::Mul : BinaryOp::Div;
    left = Expr::make_binary(std::move(left), bop, std::move(right));
  }
  return left;
}

// factor := ("-" factor) | primary
ExprPtr Parser::factor() {
  if (match(TokenKind::Minus)) {
    auto rhs = factor();
    return Expr::make_unary(UnaryOp::Negate, std::move(rhs));
  }
  return primary();
}

// primary := NUMBER | "(" expr ")"
ExprPtr Parser::primary() {
  if (check(TokenKind::Number)) {
    // Convert to double
    char* end = nullptr;
    double val = std::strtod(current.lexeme.c_str(), &end);
    if (end == current.lexeme.c_str()) {
      throw std::runtime_error(pos_str(filename, current) + "parse error: invalid number");
    }
    advance();
    return Expr::make_literal(val);
  }
  if (match(TokenKind::LParen)) {
    auto e = expression();
    expect(TokenKind::RParen, "')'");
    return Expr::make_grouping(std::move(e));
  }
  throw std::runtime_error(pos_str(filename, current) + "parse error: expected number or '('");
}

} // namespace rivet