#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"
#include "eval.hpp"
#include "rivet/token.hpp"

using namespace rivet;

static std::string to_string_value(const Value& v) {
  if (is_number(v))  { std::ostringstream os; os << as_number(v); return os.str(); }
  if (is_bool(v))    return as_bool(v) ? "true" : "false";
  if (is_string(v))  return as_string(v);
  if (is_array(v)) {
    const auto& arr = *as_array(v);
    std::string s = "[";
    for (size_t i = 0; i < arr.items.size(); ++i) {
      if (i) s += ", ";
      s += to_string_value(arr.items[i]);
    }
    s += "]";
    return s;
  }
  return "<unknown>";
}

static std::string slurp_file(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("Could not open file: " + path);
  std::ostringstream ss; ss << in.rdbuf(); return ss.str();
}

static int run_file(const std::string& path) {
  Env env; env.push();
  Parser p(slurp_file(path), path);
  Program prog = p.parse_program();
  auto last = exec_program(prog, env);
  if (last.has_value()) {
    std::cout << to_string_value(*last) << "\n";
  }
  return 0;
}

static int repl() {
  std::cout << "Rivet REPL — statements/expressions — Ctrl+C to exit\n";
  Env env; env.push();
  std::string line;
  while (true) {
    std::cout << "rvt> " << std::flush;
    if (!std::getline(std::cin, line)) break;
    if (line.empty()) continue;
    try {
      Parser p(line + "\n", "<stdin>");
      auto stmt = p.parse_one_stmt();
      auto out  = exec_stmt(*stmt, env);
      if (out.has_value()) {
        std::cout << to_string_value(*out) << "\n";
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
    }
  }
  return 0;
}

int main(int argc, char** argv) {
  try {
    if (argc == 1) return repl();
    std::string cmd = argv[1];
    if (cmd == "run" && argc >= 3) {
      return run_file(argv[2]);
    }
    std::cerr << "Usage:\n"
              << "  rvt           # REPL (statements + expressions)\n"
              << "  rvt run <file.rvt>\n";
    return 2;
  } catch (const std::exception& e) {
    std::cerr << "fatal: " << e.what() << "\n";
    return 111;
  }
}