#pragma once
#include <string>
#include <string_view>
#include <cstdint>

namespace rivet {

enum class TokenKind : uint16_t {
  // Special
  End, Error,

  // Literals & identifiers
  Identifier, Number, String,

  // Keywords
  KwLet, KwVar, KwFn, KwIf, KwElse, KwWhile, KwFor, KwReturn,
  KwTrue, KwFalse, KwNil,

  // Punctuation / operators
  LParen, RParen, LBrace, RBrace, LBracket, RBracket,
  Comma, Dot, Colon, Semicolon,

  Plus, Minus, Star, Slash, Percent,
  Bang, BangEqual,
  Equal, EqualEqual,
  Less, LessEqual,
  Greater, GreaterEqual,
  Arrow, // "->"
};

struct SourcePos {
  int line = 1;
  int col  = 1;
};

struct Token {
  TokenKind kind {TokenKind::End};
  std::string lexeme;   // raw slice copied for simplicity
  SourcePos   pos {};   // start position of token
};

inline const char* to_string(TokenKind k) {
  switch (k) {
    case TokenKind::End: return "End";
    case TokenKind::Error: return "Error";
    case TokenKind::Identifier: return "Identifier";
    case TokenKind::Number: return "Number";
    case TokenKind::String: return "String";
    case TokenKind::KwLet: return "let";
    case TokenKind::KwVar: return "var";
    case TokenKind::KwFn: return "fn";
    case TokenKind::KwIf: return "if";
    case TokenKind::KwElse: return "else";
    case TokenKind::KwWhile: return "while";
    case TokenKind::KwFor: return "for";
    case TokenKind::KwReturn: return "return";
    case TokenKind::KwTrue: return "true";
    case TokenKind::KwFalse: return "false";
    case TokenKind::KwNil: return "nil";
    case TokenKind::LParen: return "(";
    case TokenKind::RParen: return ")";
    case TokenKind::LBrace: return "{";
    case TokenKind::RBrace: return "}";
    case TokenKind::LBracket: return "[";
    case TokenKind::RBracket: return "]";
    case TokenKind::Comma: return ",";
    case TokenKind::Dot: return ".";
    case TokenKind::Colon: return ":";
    case TokenKind::Semicolon: return ";";
    case TokenKind::Plus: return "+";
    case TokenKind::Minus: return "-";
    case TokenKind::Star: return "*";
    case TokenKind::Slash: return "/";
    case TokenKind::Percent: return "%";
    case TokenKind::Bang: return "!";
    case TokenKind::BangEqual: return "!=";
    case TokenKind::Equal: return "=";
    case TokenKind::EqualEqual: return "==";
    case TokenKind::Less: return "<";
    case TokenKind::LessEqual: return "<=";
    case TokenKind::Greater: return ">";
    case TokenKind::GreaterEqual: return ">=";
    case TokenKind::Arrow: return "->";
  }
  return "Unknown";
}

} // namespace rivet