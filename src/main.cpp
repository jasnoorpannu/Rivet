#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"
#include "eval.hpp"
#include "rivet/token.hpp"

using namespace rivet;

static std::string slurp_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Could not open file: " + path);
  std::ostringstream ss; ss << in.rdbuf(); return ss.str();
}

static int lex_only(const std::string& code, const std::string& filename) {
  Lexer lx(code, filename);
  for (;;) {
    Token t = lx.next();
    if (t.kind == TokenKind::Error) {
      std::cerr << filename << ":" << t.pos.line << ":" << t.pos.col
                << ": lex error: " << t.lexeme << "\n";
      return 1;
    }
    if (t.kind == TokenKind::End) break;
    std::cout << t.pos.line << ":" << t.pos.col
              << "  " << to_string(t.kind)
              << "  \"" << t.lexeme << "\"\n";
  }
  return 0;
}

static int eval_expr_str(const std::string& code, const std::string& filename) {
  Parser p(code, filename);
  auto expr = p.parse_expression();
  double out = eval_expr(*expr);
  // Print with minimal trailing zeros
  std::cout << out << "\n";
  return 0;
}

static int repl(bool lex_mode) {
  std::cout << "Rivet REPL — " << (lex_mode ? "lexer mode" : "eval mode") << " — Ctrl+C to exit\n";
  std::string line;
  while (true) {
    std::cout << "rvt> " << std::flush;
    if (!std::getline(std::cin, line)) break;
    if (line.empty()) continue;
    try {
      if (lex_mode) {
        lex_only(line + "\n", "<stdin>");
      } else {
        eval_expr_str(line + "\n", "<stdin>");
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  try {
    if (argc == 1) {
      return repl(/*lex_mode=*/false);
    }
    std::string cmd = argv[1];
    if (cmd == "--lex") {
      if (argc == 2) return repl(true);
      if (argc >= 3) return lex_only(slurp_file(argv[2]), argv[2]);
    } else if (cmd == "run" && argc >= 3) {
      return eval_expr_str(slurp_file(argv[2]), argv[2]);
    }
    std::cerr << "Usage:\n"
              << "  rvt           # REPL (evaluate expressions)\n"
              << "  rvt --lex     # REPL (print tokens)\n"
              << "  rvt --lex <file.rvt>\n"
              << "  rvt run <file.rvt>\n";
    return 2;
  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 111;
  }
}