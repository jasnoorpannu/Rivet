#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.hpp"
#include "rivet/token.hpp"

using namespace rivet;

static std::string slurp_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Could not open file: " + path);
  std::ostringstream ss;
  ss << in.rdbuf();
  return ss.str();
}

static int run_lex(const std::string& code, const std::string& filename) {
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

static int repl() {
  std::cout << "Rivet REPL (lexer demo) — type Ctrl+C to exit\n";
  std::string line, buf;
  for (;;) {
    std::cout << "rvt> " << std::flush;
    if (!std::getline(std::cin, line)) break;
    buf = line + "\n";
    int rc = run_lex(buf, "<stdin>");
    if (rc != 0) return rc;
  }
  return 0;
}

int main(int argc, char** argv) {
  try {
    if (argc == 1) {
      return repl();
    }
    // Simple CLI:
    //   rvt run <file>  — will lex and print tokens (parser comes next)
    std::string cmd = argv[1];
    if (cmd == "run" && argc >= 3) {
      std::string path = argv[2];
      auto code = slurp_file(path);
      return run_lex(code, path);
    }
    std::cerr << "Usage:\n"
              << "  rvt            # start REPL (lexer demo)\n"
              << "  rvt run <file> # lex and print tokens\n";
    return 2;
  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 111;
  }
}