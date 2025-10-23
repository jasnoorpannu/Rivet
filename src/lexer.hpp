#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include "rivet/token.hpp"

namespace rivet {

class Lexer {
public:
  explicit Lexer(std::string source, std::string filename = "<stdin>");

  Token next();
  bool  is_at_end() const { return m_index >= m_src.size(); }

private:
  // Core
  char  peek() const;
  char  peek_next() const;
  char  advance();
  bool  match(char expected);
  void  newline();

  void  skip_space_and_comments();

  Token make_token(TokenKind kind, std::string_view text);
  Token error_token(const std::string& msg);

  Token string();
  Token number();
  Token identifier_or_keyword();

  static TokenKind keyword_kind(std::string_view ident);

private:
  std::string m_src;
  std::string m_filename;
  size_t      m_index {0};
  int         m_line  {1};
  int         m_col   {1};
};

} // namespace rivet