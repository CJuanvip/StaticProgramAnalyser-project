#pragma once
#include <StatementExtractors.h>
#include <AST.h>
/// This help traverse Variable and Constants contained in Statements
/// Which in turn help populate Name, NameType table
namespace ast {
  void extractName(const common::Var&, pkb::PKBTableManager&);
  void extractName(const common::Const&, pkb::PKBTableManager&);
  void extractNames(const common::Expr&, pkb::PKBTableManager&);
  void extractNames(const common::CondExpr&, pkb::PKBTableManager&);
  class NameExtractor : public StatementVisitorBase {
  public:
    NameExtractor(pkb::PKBTableManager& mgr)
    : mgr_(mgr) { }
    void operator() (const common::Read& o) {
      extractName(o.variable, mgr_);
    }
    void operator() (const common::Print& o) {
      extractName(o.variable, mgr_);
    }
    void operator() (const common::Call& o) {
      verifyExistence(o.procedure);
    }
    void operator() (const common::Assign& o) {
      extractName(o.variable, mgr_);
      extractNames(*o.value, mgr_);
    }
    void operator() (const common::While& o) {
      extractNames(o.condition, mgr_);
      for (const auto& s: o.statements) {
        std::visit(*this, s);
      }
    }
    void operator() (const common::If& o) {
      extractNames(o.condition, mgr_);
      for (const auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for (const auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
    }
  private:
    void verifyExistence(const common::Proc&);
    pkb::PKBTableManager& mgr_;
  };
}