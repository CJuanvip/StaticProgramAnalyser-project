#include <string>
#include "AST.h"
#include "Visitor.h"
#include "query_evaluator/Relation.h"

namespace common {

Var::Var(const std::string& n) : name(n) {}
Const::Const(const std::string& v) : value(v) {}

Add::Add(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Sub::Sub(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Mul::Mul(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Div::Div(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Mod::Mod(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}

Gt::Gt(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Lt::Lt(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Gte::Gte(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Lte::Lte(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Eq::Eq(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}
Neq::Neq(const Expr& l, const Expr& r) {
  left.reset(new Expr(l));
  right.reset(new Expr(r));
}

Not::Not(const CondExpr& l) {
  left.reset(new CondExpr(l));
}
And::And(const CondExpr& l, const CondExpr& r) {
  left.reset(new CondExpr(l));
  right.reset(new CondExpr(r));
}
Or::Or(const CondExpr& l, const CondExpr& r) {
  left.reset(new CondExpr(l));
  right.reset(new CondExpr(r));
}

Proc::Proc(const std::string& n) : name(n) {}

Read::Read(const Var& v) : variable(v) {}
Print::Print(const Var& v) : variable(v) {}
Call::Call(const Proc& p) : procedure(p) {}
While::While(const CondExpr& c, const StmtLst& s) : condition(c), statements(s) {}
If::If(const CondExpr& c, const StmtLst& t, const StmtLst& e) : condition(c), thenStatements(t), elseStatements(e) {}
Assign::Assign(const Var& var, const Expr& val) : variable(var) {
  value.reset(new Expr(val));
}

ProcDef::ProcDef(const std::string& n, const StmtLst& s) : name(n), statements(s) {}

std::string toString(const Expr& e) {
  return std::visit(visitor {
    [](Add a){ return "(" + toString(*a.left) + "+" + toString(*a.right) + ")"; },
    [](Sub a){ return "(" + toString(*a.left) + "-" + toString(*a.right) + ")"; },
    [](Mul a){ return "(" + toString(*a.left) + "*" + toString(*a.right) + ")"; },
    [](Div a){ return "(" + toString(*a.left) + "/" + toString(*a.right) + ")"; },
    [](Mod a){ return "(" + toString(*a.left) + "%" + toString(*a.right) + ")"; },
    [](Var c){ return "(" + c.name + ")"; },
    [](Const l){ return "(" + l.value + ")"; }
  }, e);
}

std::string toString(const RelExpr& e) {
  return std::visit(visitor {
    [](Gt a){ return "(" + toString(*a.left) + ">" + toString(*a.right) + ")"; },
    [](Lt a){ return "(" + toString(*a.left) + "<" + toString(*a.right) + ")"; },
    [](Gte a){ return "(" + toString(*a.left) + ">=" + toString(*a.right) + ")"; },
    [](Lte a){ return "(" + toString(*a.left) + "<=" + toString(*a.right) + ")"; },
    [](Eq a){ return "(" + toString(*a.left) + "==" + toString(*a.right) + ")"; },
    [](Neq a){ return "(" + toString(*a.left) + "!=" + toString(*a.right) + ")"; }
  }, e);
}

std::string toString(const CondExpr& e) {
  return std::visit(visitor {
    [](Not a){ return "!(" + toString(*a.left) + ")"; },
    [](And a){ return "(" + toString(*a.left) + "&&" + toString(*a.right) + ")"; },
    [](Or a){ return "(" + toString(*a.left) + "||" + toString(*a.right) + ")"; },
    [](RelExpr a){ return toString(a); }
  }, e);
}

std::string toString(const StmtLst& l) {
  std::string str;
  for (const auto& stmt : l) str += toString(stmt) + "\n";
  return str;
}

std::string toString(const Proc& p) {
  return p.name;
}

std::string toString(const Stmt& s) {
  return std::visit(visitor {
    [](While a){ return "while (" + toString(a.condition) + ") {\n" + toString(a.statements) + "}"; },
    [](If a){ return "if (" + toString(a.condition) + ") then {\n" + toString(a.thenStatements) + "} else {\n" + toString(a.elseStatements) + "}"; },
    [](Assign a){ return toString(a.variable) + " = " + toString(*a.value) + ";"; },
    [](Read a){ return "read " + toString(a.variable) + ";"; },
    [](Print a){ return "print " + toString(a.variable) + ";"; },
    [](Call a){ return "call " + toString(a.procedure) + ";"; }
  }, s);
}

std::string toString(const Var& v) {
  return v.name;
}

std::string toString(const ProcDef& p) {
  return "procedure " + p.name + " {\n" + toString(p.statements) + "}";
}

std::string toString(const Program& p) {
  std::string str;
  for (const auto& def : p) str += toString(def) + "\n";
  return str;
}

std::string toString(const StmtRef& s) {
  return std::visit(visitor {
    [](Synonym a){ return a; },
    [](Wildcard) -> std::string { return "_"; },
    [](Int a){ return a.value; }
  }, s);
}

std::string toString(const EntRef& s) {
  return std::visit(visitor {
    [](Synonym a){ return a; },
    [](Wildcard){ return std::string("_"); },
    [](Var a){ return "\"" + a.name + "\""; }
  }, s);
}

std::string toString(const SuchThat& s) {
  return std::visit(visitor {
    [&](StmtStmtRelation a){ return RELATION_ID_STRING.at(a.type) + "(" + toString(a.left) + ", " + toString(a.right) + ")"; },
    [&](StmtEntRelation a){ return RELATION_ID_STRING.at(a.type) + "(" + toString(a.left) + ", " + toString(a.right) + ")"; },
    [&](EntEntRelation a){ return RELATION_ID_STRING.at(a.type) + "(" + toString(a.left) + ", " + toString(a.right) + ")"; },
    [&](StmtEntEntRelation a){
    return RELATION_ID_STRING.at(a.type) + "(" +
    std::visit(visitor {[](auto a){ return toString(a); }}, a.left) +
    ", " + toString(a.right) + ")";
    }
  }, s);
}

std::string toString(const PatternRHS& p) {
  return std::visit(visitor {
    [](ExactMatch a){ return "\"" + toString(a.expr) + "\""; },
    [](PartialMatch a){ return "_\"" + toString(a.expr) + "\"_"; },
    [](Wildcard){ return std::string("_"); }
  }, p);
}

std::string toString(const Pattern& p) {
  return std::visit(visitor {
    [](BinaryPattern a){ return a.synonym + " (" + toString(a.left) + ", " + toString(a.right) + ")"; },
    [](TernaryPattern a){ return a.synonym + " (" + toString(a.left) + ", _, _)"; }
  }, p);
}

std::string toString(const AttrRef& r) {
  if (r.attribute) return r.synonym + "." + ATTRIBUTE_ID_STRING.at(*r.attribute);
  return r.synonym;
}

std::string toString(const Ref& r) {
  return std::visit(visitor {
    [](std::string a){ return a; },
    [](Int a){ return a.value; },
    [](AttrRef a){ return toString(a); },
  }, r);
}

std::string toString(const Declaration& d) {
  return TYPE_ID_STRING.at(d.type) + " " + d.synonym;
}

std::string toString(const Clause& c) {
  return visit(visitor {
    [](SuchThat a){ return "such that " + toString(a); },
    [](Pattern a){ return "pattern " + toString(a); },
    [](With a){ return "with " + toString(a.first) + " = " + toString(a.second); }
  }, c);
}

std::string toString(const std::vector<AttrRef>& r) {
  if (r.size() == 1) return toString(r[0]);
  std::string ret = toString(r[0]);
  for (size_t i=1; i < r.size(); i++) ret += ", " + toString(r[i]);
  return std::string("<") + ret + ">";
}

std::string toString(const Query& q) {
  std::string ret;
  for (const auto& d : q.declarations) ret += toString(d) + "; ";
  ret += "\n";
  ret += "Select ";
  if (q.isBooleanResult) {
    ret += "BOOLEAN\n";
  } else {
    ret += toString(q.result) + "\n";
  }
  for (const auto& s : q.suchThats) ret += toString(s) + " ";
  ret += "\n";
  for (const auto& p : q.patterns) ret += toString(p) + " ";
  ret += "\n";
  for (const auto& w : q.withs) ret += toString(w) + " ";
  return ret;
}

std::string toString(const Const& constant) {
  return constant.value;
}

}
