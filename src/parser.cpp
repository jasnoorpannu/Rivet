#include "parser.hpp"
#include <stdexcept>
#include <sstream>
#include <cstdlib>

namespace rivet {

static std::string pos_str(const std::string& file, const Token& t) {
  std::ostringstream oss; oss << file << ":" << t.pos.line << ":" << t.pos.col << ": "; return oss.str();
}

Parser::Parser(std::string source, std::string filename_)
  : lex(std::move(source), filename_), filename(std::move(filename_)) {
  current = lex.next();
  if (current.kind == TokenKind::Error) throw std::runtime_error(pos_str(filename, current) + "lex error: " + current.lexeme);
}

const Token& Parser::advance() {
  current = lex.next();
  if (current.kind == TokenKind::Error) throw std::runtime_error(pos_str(filename, current) + "lex error: " + current.lexeme);
  return current;
}
bool Parser::match(TokenKind k) { if (check(k)) { advance(); return true; } return false; }
const Token& Parser::expect(TokenKind k, const char* msg) {
  if (!check(k)) throw std::runtime_error(pos_str(filename, current) + std::string("parse error: expected ") + msg);
  return advance();
}

Program Parser::parse_program() { Program p; while (!check(TokenKind::End)) p.push_back(statement()); return p; }
StmtPtr Parser::parse_one_stmt() { auto s=statement(); if(!check(TokenKind::End)) throw std::runtime_error(pos_str(filename,current)+"parse error: expected end of input"); return s; }

// ----------------- Statements -----------------
StmtPtr Parser::statement() {
  if (check(TokenKind::KwLet))    return let_stmt();
  if (check(TokenKind::KwVar))    return var_stmt();
  if (check(TokenKind::KwIf))     return if_stmt();
  if (check(TokenKind::KwWhile))  return while_stmt();
  if (check(TokenKind::KwFor))    return for_stmt();
  if (check(TokenKind::KwPrint))  return print_stmt();
  if (check(TokenKind::KwFn))     return fn_decl();
  if (check(TokenKind::KwReturn)) return return_stmt();
  if (check(TokenKind::LBrace))   return block_stmt();
  return assign_or_expr_stmt();
}

StmtPtr Parser::let_stmt() {
  expect(TokenKind::KwLet, "'let'");
  if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected identifier");
  std::string name = current.lexeme; advance();
  expect(TokenKind::Equal, "'='");
  auto init = expression();
  if (check(TokenKind::Semicolon)) advance();
  return Stmt::make_let(std::move(name), std::move(init));
}

StmtPtr Parser::var_stmt() {
  expect(TokenKind::KwVar, "'var'");
  if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected identifier");
  std::string name = current.lexeme; advance();
  expect(TokenKind::Equal, "'='");
  auto init = expression();
  if (check(TokenKind::Semicolon)) advance();
  return Stmt::make_var(std::move(name), std::move(init));
}

StmtPtr Parser::assign_or_expr_stmt() {
  if (check(TokenKind::Identifier)) {
    Token ident = current; advance();
    if (check(TokenKind::Equal)) {
      advance();
      auto rhs = expression();
      if (check(TokenKind::Semicolon)) advance();
      return Stmt::make_assign(ident.lexeme, std::move(rhs));
    } else if (check(TokenKind::LParen)) {
      advance();
      std::vector<ExprPtr> args;
      if (!check(TokenKind::RParen)) args = arg_list();
      expect(TokenKind::RParen, "')'");
      if (check(TokenKind::Semicolon)) advance();
      return Stmt::make_expr(Expr::make_call(ident.lexeme, std::move(args)));
    } else {
      auto e = Expr::make_variable(ident.lexeme);
      if (check(TokenKind::Semicolon)) advance();
      return Stmt::make_expr(std::move(e));
    }
  }
  auto e = expression();
  if (check(TokenKind::Semicolon)) advance();
  return Stmt::make_expr(std::move(e));
}

StmtPtr Parser::block_stmt() {
  expect(TokenKind::LBrace, "'{'");
  std::vector<StmtPtr> ss;
  while (!check(TokenKind::RBrace)) {
    if (check(TokenKind::End)) throw std::runtime_error(pos_str(filename, current) + "parse error: unterminated block");
    ss.push_back(statement());
  }
  expect(TokenKind::RBrace, "'}'");
  return Stmt::make_block(std::move(ss));
}

StmtPtr Parser::if_stmt() {
  expect(TokenKind::KwIf, "'if'");
  expect(TokenKind::LParen, "'('");
  auto c = expression();
  expect(TokenKind::RParen, "')'");
  auto t = statement();
  StmtPtr e; if (match(TokenKind::KwElse)) e = statement(); else e = Stmt::make_block({});
  return Stmt::make_if(std::move(c), std::move(t), std::move(e));
}

StmtPtr Parser::while_stmt() {
  expect(TokenKind::KwWhile, "'while'");
  expect(TokenKind::LParen, "'('");
  auto c = expression();
  expect(TokenKind::RParen, "')'");
  auto b = statement();
  return Stmt::make_while(std::move(c), std::move(b));
}

// ---- for helpers already declared in parser.hpp ----
StmtPtr Parser::let_decl_no_semi() {
  expect(TokenKind::KwLet, "'let'");
  if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected identifier");
  std::string name = current.lexeme; advance();
  expect(TokenKind::Equal, "'='");
  auto init = expression();
  return Stmt::make_let(std::move(name), std::move(init));
}
StmtPtr Parser::var_decl_no_semi() {
  expect(TokenKind::KwVar, "'var'");
  if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected identifier");
  std::string name = current.lexeme; advance();
  expect(TokenKind::Equal, "'='");
  auto init = expression();
  return Stmt::make_var(std::move(name), std::move(init));
}
StmtPtr Parser::assign_or_expr_no_semi() {
  if (check(TokenKind::Identifier)) {
    Token ident = current; advance();
    if (check(TokenKind::Equal)) {
      advance();
      auto rhs = expression();
      return Stmt::make_assign(ident.lexeme, std::move(rhs));
    } else if (check(TokenKind::LParen)) {
      advance();
      std::vector<ExprPtr> args; if (!check(TokenKind::RParen)) args = arg_list();
      expect(TokenKind::RParen, "')'");
      return Stmt::make_expr(Expr::make_call(ident.lexeme, std::move(args)));
    } else {
      return Stmt::make_expr(Expr::make_variable(ident.lexeme));
    }
  }
  auto e = expression();
  return Stmt::make_expr(std::move(e));
}

// 'for' '(' init? ';' cond? ';' step? ')' stmt    OR   'for' ident 'in' expr stmt
StmtPtr Parser::for_stmt() {
  expect(TokenKind::KwFor, "'for'");

  // for-in (no parentheses): for x in expr stmt
  if (!check(TokenKind::LParen)) {
    if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected identifier after 'for'");
    std::string var = current.lexeme; advance();
    expect(TokenKind::KwIn, "'in'");
    auto it = expression();
    auto body = statement();
    return Stmt::make_for_in(std::move(var), std::move(it), std::move(body));
  }

  // C-style
  advance(); // '('
  StmtPtr init;
  if (!check(TokenKind::Semicolon)) {
    if (check(TokenKind::KwLet))      init = let_decl_no_semi();
    else if (check(TokenKind::KwVar)) init = var_decl_no_semi();
    else                              init = assign_or_expr_no_semi();
  }
  expect(TokenKind::Semicolon, "';'");

  ExprPtr cond;
  if (!check(TokenKind::Semicolon)) cond = expression();
  expect(TokenKind::Semicolon, "';'");

  StmtPtr step;
  if (!check(TokenKind::RParen)) step = assign_or_expr_no_semi();
  expect(TokenKind::RParen, "')'");

  auto body = statement();
  return Stmt::make_for_c(std::move(init), std::move(cond), std::move(step), std::move(body));
}

// ----------------- rest unchanged (print/fn/return + expressions) -----------------
StmtPtr Parser::print_stmt() {
  expect(TokenKind::KwPrint, "'print'");
  auto e = expression();
  if (check(TokenKind::Semicolon)) advance();
  return Stmt::make_print(std::move(e));
}

StmtPtr Parser::fn_decl() {
  expect(TokenKind::KwFn, "'fn'");
  if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected function name");
  std::string name = current.lexeme; advance();
  expect(TokenKind::LParen, "'('");
  std::vector<std::string> params;
  if (!check(TokenKind::RParen)) {
    do {
      if (!check(TokenKind::Identifier)) throw std::runtime_error(pos_str(filename, current) + "parse error: expected parameter name");
      params.push_back(current.lexeme); advance();
    } while (match(TokenKind::Comma));
  }
  expect(TokenKind::RParen, "')'");
  auto body = block_stmt();
  return Stmt::make_fn(std::move(name), std::move(params), std::move(body));
}

StmtPtr Parser::return_stmt() {
  expect(TokenKind::KwReturn, "'return'");
  ExprPtr v; if (!check(TokenKind::Semicolon) && !check(TokenKind::End) && !check(TokenKind::RBrace)) v = expression();
  else v = Expr::make_number(0.0);
  if (check(TokenKind::Semicolon)) advance();
  return Stmt::make_return(std::move(v));
}

// ================== Expressions (same as before) ==================
ExprPtr Parser::expression() { return or_expr(); }
ExprPtr Parser::or_expr() { auto l=and_expr(); while(check(TokenKind::OrOr)){advance(); auto r=and_expr(); l=Expr::make_binary(std::move(l), BinaryOp::LOr, std::move(r));} return l; }
ExprPtr Parser::and_expr(){ auto l=equality(); while(check(TokenKind::AndAnd)){advance(); auto r=equality(); l=Expr::make_binary(std::move(l), BinaryOp::LAnd, std::move(r));} return l; }
ExprPtr Parser::equality(){ auto l=comparison(); while(check(TokenKind::EqualEqual)||check(TokenKind::BangEqual)){Token op=current; advance(); auto r=comparison(); auto bop=(op.kind==TokenKind::EqualEqual)?BinaryOp::Eq:BinaryOp::Ne; l=Expr::make_binary(std::move(l), bop, std::move(r));} return l; }
ExprPtr Parser::comparison(){ auto l=term(); while(check(TokenKind::Less)||check(TokenKind::LessEqual)||check(TokenKind::Greater)||check(TokenKind::GreaterEqual)){Token op=current; advance(); auto r=term(); BinaryOp bop; switch(op.kind){case TokenKind::Less:bop=BinaryOp::Lt;break;case TokenKind::LessEqual:bop=BinaryOp::Le;break;case TokenKind::Greater:bop=BinaryOp::Gt;break;default:bop=BinaryOp::Ge;} l=Expr::make_binary(std::move(l), bop, std::move(r)); } return l; }
ExprPtr Parser::term(){ auto l=factor(); while(check(TokenKind::Plus)||check(TokenKind::Minus)){Token op=current; advance(); auto r=factor(); auto bop=(op.kind==TokenKind::Plus)?BinaryOp::Add:BinaryOp::Sub; l=Expr::make_binary(std::move(l), bop, std::move(r));} return l; }
ExprPtr Parser::factor(){ auto l=unary(); while(check(TokenKind::Star)||check(TokenKind::Slash)){Token op=current; advance(); auto r=unary(); auto bop=(op.kind==TokenKind::Star)?BinaryOp::Mul:BinaryOp::Div; l=Expr::make_binary(std::move(l), bop, std::move(r));} return l; }
ExprPtr Parser::unary(){ if(check(TokenKind::Minus)){advance(); return Expr::make_unary(UnaryOp::Negate, unary());} if(check(TokenKind::Bang)){advance(); return Expr::make_unary(UnaryOp::Not, unary());} return call(); }
ExprPtr Parser::call(){ if(check(TokenKind::Identifier)){Token ident=current; advance(); if(check(TokenKind::LParen)){advance(); std::vector<ExprPtr> args; if(!check(TokenKind::RParen)) args=arg_list(); expect(TokenKind::RParen, "')'"); return Expr::make_call(ident.lexeme, std::move(args));} return Expr::make_variable(ident.lexeme);} return primary(); }
std::vector<ExprPtr> Parser::arg_list(){ std::vector<ExprPtr> args; args.push_back(expression()); while(match(TokenKind::Comma)) args.push_back(expression()); return args; }
ExprPtr Parser::array_lit(){ expect(TokenKind::LBracket, "'['"); std::vector<ExprPtr> elems; if(!check(TokenKind::RBracket)){ elems.push_back(expression()); while(match(TokenKind::Comma)) elems.push_back(expression()); } expect(TokenKind::RBracket, "']'"); return Expr::make_array(std::move(elems)); }
ExprPtr Parser::primary(){
  if (check(TokenKind::Number)) { char* end=nullptr; double v=std::strtod(current.lexeme.c_str(), &end); if (end==current.lexeme.c_str()) throw std::runtime_error(pos_str(filename,current)+"parse error: invalid number"); advance(); return Expr::make_number(v); }
  if (check(TokenKind::KwTrue))  { advance(); return Expr::make_bool(true); }
  if (check(TokenKind::KwFalse)) { advance(); return Expr::make_bool(false); }
  if (check(TokenKind::String))  { std::string s=current.lexeme; advance(); return Expr::make_string(std::move(s)); }
  if (check(TokenKind::LBracket)) return array_lit();
  if (match(TokenKind::LParen))   { auto e=expression(); expect(TokenKind::RParen, "')'"); return Expr::make_grouping(std::move(e)); }
  throw std::runtime_error(pos_str(filename, current) + "parse error: expected expression");
}

} // namespace rivet