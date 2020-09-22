#include <ParentExtractor.h>
namespace ast {
  using namespace common;
  struct ParentExtractor {
    ParentExtractor(pkb::PKBTableManager& mgr, std::vector<unsigned>& enclosingStatements)
      : parentTable_(mgr.getParent())
      , parentClosureTable_(mgr.getParentClosure())
      , enclosingStmts_(enclosingStatements) { }

    void operator() (const While& o) {
      populateTables(o.statementId);

      // Add this node to enclosingStmts_, and recursively call on nested nodes
      enclosingStmts_.push_back(o.statementId); 
      for (const auto& statement: o.statements) {
        std::visit(*this, statement);
      }
      enclosingStmts_.pop_back();
    }
    void operator() (const If& o) {
      populateTables(o.statementId);

      // Add this node to enclosingStmts_, and recursively call on nested nodes
      enclosingStmts_.push_back(o.statementId); 
      for (const auto& statement: o.thenStatements) {
        std::visit(*this, statement);
      }
      for (const auto& statement: o.elseStatements) {
        std::visit(*this, statement);
      }
      enclosingStmts_.pop_back();
    }

    void operator() (const Call& o) {
      populateTables(o.statementId);
    }
    void operator() (const Read& o) {
      populateTables(o.statementId);
    }
    void operator() (const Print& o) {
      populateTables(o.statementId);
    }
    void operator() (const Assign& o) {
      populateTables(o.statementId);
    }

  private:
    void populateTables(unsigned statementId) {
            // check for empty path
      if (!enclosingStmts_.empty()) {
        unsigned parentId = enclosingStmts_.back(); 
        parentTable_.insert(parentId, statementId);
      }
      // Populate parent closure table
      for (const auto ancestorId : enclosingStmts_) {
        parentClosureTable_.insert(ancestorId, statementId);
      }
    }
    pkb::PKBTable<unsigned, unsigned>& parentTable_;
    pkb::PKBTable<unsigned, unsigned>& parentClosureTable_;
    std::vector<unsigned>& enclosingStmts_; // acts like a path to a file, the most recent entry is the direct parent
  };
  void extractParent(const StmtLst& statements, pkb::PKBTableManager& mgr) {
    std::vector<unsigned> path;
    ParentExtractor extractor(mgr, path);
    auto recurseTransferParent = [&extractor](const Stmt& statement) {
      std::visit(extractor, statement);
    };

    for (const auto& statement: statements) {
      recurseTransferParent(statement);
    }
  }
  void extractParent(const ProcDef& procedure, pkb::PKBTableManager& mgr) {
    extractParent(procedure.statements, mgr);
  }
}