#pragma once
#include <variant>
#include <string>
#include <vector>
#include <memory>
#include "common/RelationId.h"
#include "common/EntityId.h"

namespace common {

struct Add;
struct Sub;
struct Mul;
struct Div;
struct Mod;
struct Var;
struct Const;
using Expr = std::variant<Add,Sub,Mul,Div,Mod,Var,Const>;

struct Var {
  std::string name;
  Var() = default;
  explicit Var(const std::string& n);
};
struct Const {
  std::string value;
  Const() = default;
  explicit Const(const std::string& v);
};

struct Add {
  std::shared_ptr<Expr> left, right;
  Add() = default;
  Add(const Expr& l, const Expr& r);
};
struct Sub {
  std::shared_ptr<Expr> left, right;
  Sub() = default;
  Sub(const Expr& l, const Expr& r);
};
struct Mul {
  std::shared_ptr<Expr> left, right;
  Mul() = default;
  Mul(const Expr& l, const Expr& r);
};
struct Div {
  std::shared_ptr<Expr> left, right;
  Div() = default;
  Div(const Expr& l, const Expr& r);
};
struct Mod {
  std::shared_ptr<Expr> left, right;
  Mod() = default;
  Mod(const Expr& l, const Expr& r);
};

struct Gt {
  std::shared_ptr<Expr> left, right;
  Gt() = default;
  Gt(const Expr& l, const Expr& r);
};
struct Lt {
  std::shared_ptr<Expr> left, right;
  Lt() = default;
  Lt(const Expr& l, const Expr& r);
};
struct Gte {
  std::shared_ptr<Expr> left, right;
  Gte() = default;
  Gte(const Expr& l, const Expr& r);
};
struct Lte {
  std::shared_ptr<Expr> left, right;
  Lte() = default;
  Lte(const Expr& l, const Expr& r);
};
struct Eq {
  std::shared_ptr<Expr> left, right;
  Eq() = default;
  Eq(const Expr& l, const Expr& r);
};
struct Neq {
  std::shared_ptr<Expr> left, right;
  Neq() = default;
  Neq(const Expr& l, const Expr& r);
};
using RelExpr = std::variant<Gt,Lt,Gte,Lte,Eq,Neq>;

struct Not;
struct And;
struct Or;
using CondExpr = std::variant<Not,And,Or,RelExpr>;

struct Not {
  std::shared_ptr<CondExpr> left;
  Not() = default;
  explicit Not(const CondExpr& left);
};
struct And {
  std::shared_ptr<CondExpr> left, right;
  And() = default;
  And(const CondExpr& left, const CondExpr& right);
};
struct Or {
  std::shared_ptr<CondExpr> left, right;
  Or() = default;
  Or(const CondExpr& left, const CondExpr& right);
};

struct Proc {
  std::string name;
  Proc() = default;
  explicit Proc(const std::string& n);
};

struct While;
struct If;
struct Assign;
struct Read {
  Var variable;
  unsigned statementId;
  explicit Read(const Var& v);
};
struct Print {
  Var variable;
  unsigned statementId;
  explicit Print(const Var& v);
};
struct Call {
  Proc procedure;
  unsigned statementId;
  explicit Call(const Proc& p);
};
using Stmt = std::variant<While,If,Assign,Read,Print,Call>;
using StmtLst = std::vector<Stmt>;

struct While {
  CondExpr condition;
  StmtLst statements;
  unsigned statementId;
  While(const CondExpr& c, const StmtLst& s);
};
struct If {
  CondExpr condition;
  StmtLst thenStatements;
  StmtLst elseStatements;
  unsigned statementId;
  If(const CondExpr& c, const StmtLst& t, const StmtLst& e);
};
struct Assign {
  Var variable;
  std::shared_ptr<Expr> value;
  unsigned statementId;
  Assign(const Var& var, const Expr& val);
};

struct ProcDef {
  std::string name;
  StmtLst statements;
  ProcDef(const std::string& n, const StmtLst& s);
};
using Program = std::vector<ProcDef>;

std::string toString(const Expr& e);
std::string toString(const RelExpr& e);
std::string toString(const CondExpr& e);
std::string toString(const StmtLst& l);
std::string toString(const Proc& p);
std::string toString(const Stmt& s);
std::string toString(const ProcDef& p);
std::string toString(const Program& p);

using Synonym = std::string;
struct Declaration {
  EntityId type;
  Synonym synonym;
};

struct Wildcard {};
struct Int {
  bool operator==(const Int right) const {
    return value == right.value;
  }
  std::string value;
};
using StmtRef = std::variant<Synonym,Wildcard,Int>;
using EntRef = std::variant<Synonym,Wildcard,Var>;

// Follows, FollowsT, Parent, ParentT
struct StmtStmtRelation {
  RelationId type;
  StmtRef left, right;
};
// unused for now
struct StmtEntRelation {
  RelationId type;
  StmtRef left;
  EntRef right;
};
// unused for now
struct EntEntRelation {
  RelationId type;
  EntRef left, right;
};
// UsesS, UsesP, ModifiesS, ModifiesP
struct StmtEntEntRelation {
  RelationId type;
  std::variant<StmtRef,EntRef> left;
  EntRef right;
};
using SuchThat = std::variant<StmtStmtRelation,StmtEntRelation,EntEntRelation,StmtEntEntRelation>;

struct ExactMatch { Expr expr; };
struct PartialMatch { Expr expr; };
using PatternRHS = std::variant<ExactMatch,PartialMatch,Wildcard>;
struct BinaryPattern {
  Synonym synonym;
  EntRef left;
  PatternRHS right;
};
struct TernaryPattern {
  Synonym synonym;
  EntRef left;
};
using Pattern = std::variant<BinaryPattern,TernaryPattern>;

struct AttrRef {
  /* implicit */ AttrRef(Synonym synonym) : synonym(synonym) {};
  bool operator==(const AttrRef right) const {
    return synonym == right.synonym && attribute == right.attribute;
  }
  AttrRef(Synonym synonym, std::optional<AttrNameId> attribute) : synonym(synonym), attribute(attribute) {};
  Synonym synonym;
  std::optional<AttrNameId> attribute;
};
using Ref = std::variant<std::string,Int,AttrRef>;
using With = std::pair<Ref,Ref>;

using Clause = std::variant<SuchThat,Pattern,With>;

// only used by parser
struct Bool {};
using Result = std::variant<std::vector<AttrRef>,Bool>;

struct Query {
  std::vector<Declaration> declarations;
  std::vector<AttrRef> result;
  bool isBooleanResult = false;
  std::vector<SuchThat> suchThats;
  std::vector<Pattern> patterns;
  std::vector<With> withs;
};

std::string toString(const StmtRef& s);
std::string toString(const EntRef& s);
std::string toString(const Var& v);
std::string toString(const Const& constant);
std::string toString(const Declaration& d);
std::string toString(const Clause& c);
std::string toString(const Ref& r);
std::string toString(const AttrRef& r);
std::string toString(const Result& r);
std::string toString(const Query& q);

}
