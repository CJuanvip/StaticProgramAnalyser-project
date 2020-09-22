#include <FollowExtractor.h>
namespace ast {
  using namespace common;
  void extractFollow(const StmtLst&, pkb::PKBTableManager&);

  struct FollowVisitor {
    FollowVisitor(pkb::PKBTableManager& mgr) : mgr_(mgr) { }
    void operator() (const While& o) {
      extractFollow(o.statements, mgr_);
    }
    void operator() (const If& o) {
      extractFollow(o.thenStatements, mgr_);
      extractFollow(o.elseStatements, mgr_);
    }
    void operator() (const Call&) { }
    void operator() (const Read&) { }
    void operator() (const Print&) { }
    void operator() (const Assign&) { }
  private:
    pkb::PKBTableManager& mgr_;
  };

  void extractFollow(const StmtLst& statements, pkb::PKBTableManager& mgr) {
    std::vector<unsigned> statementIds;
    auto copyStatementIds = [&statementIds](const Stmt& statement) {
      std::visit([&statementIds](auto const& o) {statementIds.push_back(o.statementId); }, statement);
    };
    FollowVisitor visitor(mgr);
    auto recurseTransferFollow = [&visitor](const Stmt& statement) {
      std::visit(visitor, statement);
    };
    // we first handle the relative sequence of this statementList
    for (const auto& statement: statements) {
      copyStatementIds(statement);
      recurseTransferFollow(statement);
    }
    auto& tableFollow = mgr.getFollow();
    auto& tableFollowClosure = mgr.getFollowClosure();
    for (int i = 0; i < static_cast<int>(statementIds.size()) - 1; ++i) {
      int j = i + 1;
      tableFollow.insert(statementIds[i], statementIds[j]);
      tableFollowClosure.insert(statementIds[i], statementIds[j]);
      for (j = i + 2; j < static_cast<int>(statementIds.size()); ++j) {
        tableFollowClosure.insert(statementIds[i], statementIds[j]);
      }
    }
  }

  void extractFollow(const ProcDef& procedure, pkb::PKBTableManager& mgr) {
    extractFollow(procedure.statements, mgr);
  }
}