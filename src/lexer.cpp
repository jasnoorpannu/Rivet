#include "lexer.hpp"
#include <cctype>
#include <cassert>
#include <unordered_map>

namespace rivet {

// ---------------- core ----------------

Lexer::Lexer(std::string source, std::string filename)
  : m_src(std::move(source)), m_filename(std::move(filename)) {}

char Lexer::peek() const {
  return m_index < m_src.size() ? m_src[m_index] : '\0';
}
char Lexer::peek_next() const {
  return (m_index + 1) < m_src.size() ? m_src[m_index + 1] : '\0';
}
char Lexer::advance() {
  char c = peek();
  if (c == '\0') return '\0';
  ++m_index;
  if (c == '\n') newline(); else ++m_col;
  return c;
}
bool Lexer::match(char expected) {
  if (peek() != expected) return false;
  advance();
  return true;
}
void Lexer::newline() {
  ++m_line; m_col = 1;
}

Token Lexer::make_token(TokenKind kind, std::string_view text) {
  Token t;
  t.kind = kind;
  t.lexeme.assign(text.begin(), text.end());
  // position is start column = current col - len(text)
  t.pos = {m_line, m_col - static_cast<int>(text.size())};
  return t;
}

Token Lexer::error_token(const std::string& msg) {
  Token t;
  t.kind = TokenKind::Error;
  t.lexeme = msg;
  t.pos = {m_line, m_col};
  return t;
}

// ------------- skipping -------------

void Lexer::skip_space_and_comments() {
  for (;;) {
    char c = peek();
    if (c == ' ' || c == '\t' || c == '\r') { advance(); continue; }
    if (c == '\n') { advance(); continue; }

    // Single-line comment: //
    if (c == '/' && peek_next() == '/') {
      while (peek() != '\n' && peek() != '\0') advance();
      continue;
    }
    // Block comment: /* ... */
    if (c == '/' && peek_next() == '*') {
      advance(); advance(); // consume '/*'
      while (peek() != '\0' && !(peek() == '*' && peek_next() == '/')) {
        advance();
      }
      if (peek() == '*' && peek_next() == '/') { advance(); advance(); }
      continue;
    }
    break;
  }
}

// ------------- keywords -------------

TokenKind Lexer::keyword_kind(std::string_view s) {
  static const std::unordered_map<std::string_view, TokenKind> map = {
    {"let",    TokenKind::KwLet},
    {"var",    TokenKind::KwVar},
    {"fn",     TokenKind::KwFn},
    {"if",     TokenKind::KwIf},
    {"else",   TokenKind::KwElse},
    {"while",  TokenKind::KwWhile},
    {"for",    TokenKind::KwFor},
    {"in",     TokenKind::KwIn},
    {"return", TokenKind::KwReturn},
    {"print",  TokenKind::KwPrint},
    {"true",   TokenKind::KwTrue},
    {"false",  TokenKind::KwFalse},
    {"nil",    TokenKind::KwNil},
  };
  if (auto it = map.find(s); it != map.end()) return it->second;
  return TokenKind::Identifier;
}

// ------------- scanners -------------

Token Lexer::identifier_or_keyword() {
  size_t start = m_index;
  while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') advance();
  std::string_view text{m_src.data() + start, m_index - start};
  return make_token(keyword_kind(text), text);
}

Token Lexer::number() {
  size_t start = m_index;
  while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
  if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
    advance(); // dot
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
  }
  std::string_view text{m_src.data() + start, m_index - start};
  return make_token(TokenKind::Number, text);
}

Token Lexer::string() {
  // Consume opening quote
  char quote = advance();
  assert(quote == '"' || quote == '\'');
  size_t start = m_index;

  while (peek() != '\0' && peek() != quote) {
    if (peek() == '\\') { // escape sequence: skip next char literally
      advance();
      if (peek() != '\0') advance();
    } else {
      advance();
    }
  }
  if (peek() == '\0') {
    return error_token("Unterminated string");
  }
  size_t end = m_index;
  advance(); // closing quote

  // Store inner contents as the token's lexeme (no quotes)
  std::string_view inner{m_src.data() + start, end - start};
  return make_token(TokenKind::String, inner);
}

// ------------- main ---------------

Token Lexer::next() {
  skip_space_and_comments();
  char c = peek();
  if (c == '\0') {
    return make_token(TokenKind::End, "");
  }

  // Identifiers
  if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
    return identifier_or_keyword();
  }
  // Numbers
  if (std::isdigit(static_cast<unsigned char>(c))) {
    return number();
  }
  // Strings
  if (c == '"' || c == '\'') {
    return string();
  }

  // Single / compound operators & punctuation
  advance();
  switch (c) {
    // Brackets / braces / parens / punctuation
    case '(': return make_token(TokenKind::LParen, "(");
    case ')': return make_token(TokenKind::RParen, ")");
    case '{': return make_token(TokenKind::LBrace, "{");
    case '}': return make_token(TokenKind::RBrace, "}");
    case '[': return make_token(TokenKind::LBracket, "[");
    case ']': return make_token(TokenKind::RBracket, "]");
    case ',': return make_token(TokenKind::Comma, ",");
    case '.': return make_token(TokenKind::Dot, ".");
    case ':': return make_token(TokenKind::Colon, ":");
    case ';': return make_token(TokenKind::Semicolon, ";");

    // Math
    case '+': return make_token(TokenKind::Plus, "+");
    case '-': {
      if (match('>')) return make_token(TokenKind::Arrow, "->");
      return make_token(TokenKind::Minus, "-");
    }
    case '*': return make_token(TokenKind::Star, "*");
    case '/': return make_token(TokenKind::Slash, "/");
    case '%': return make_token(TokenKind::Percent, "%");

    // Logical and comparison
    case '!': {
      bool eq = match('=');
      return make_token(eq ? TokenKind::BangEqual : TokenKind::Bang, eq ? "!=" : "!");
    }
    case '=': {
      bool eq = match('=');
      return make_token(eq ? TokenKind::EqualEqual : TokenKind::Equal, eq ? "==" : "=");
    }
    case '<': {
      bool eq = match('=');
      return make_token(eq ? TokenKind::LessEqual : TokenKind::Less, eq ? "<=" : "<");
    }
    case '>': {
      bool eq = match('=');
      return make_token(eq ? TokenKind::GreaterEqual : TokenKind::Greater, eq ? ">=" : ">");
    }

    // && and ||
    case '&': {
      if (match('&')) return make_token(TokenKind::AndAnd, "&&");
      break; // fallthrough to error
    }
    case '|': {
      if (match('|')) return make_token(TokenKind::OrOr, "||");
      break; // fallthrough to error
    }
  }

  // Unknown character
  Token t;
  t.kind = TokenKind::Error;
  t.lexeme = std::string("Unexpected character: '") + static_cast<char>(c) + "'";
  t.pos = {m_line, m_col - 1}; // we already advanced once
  return t;
}

} // namespace rivet