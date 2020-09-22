#include <StatementIdNormalizer.h>
namespace ast {
  using namespace common;
  namespace {
  struct StatementIdNormalizer {
    StatementIdNormalizer(unsigned& nextStatementId)
    : statementId_(nextStatementId) { }

    void operator() (Read& o) {
      o.statementId = statementId_++;
    }

    void operator() (Print& o) {
      o.statementId = statementId_++;
    }

    void operator() (Call& o) {
      o.statementId = statementId_++;
    }

    void operator() (While& o) {
      o.statementId = statementId_++;
      for (auto& s: o.statements) {
        std::visit(*this, s);
      }
    }

    void operator() (If& o) {
      o.statementId = statementId_++;
      for (auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for (auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
    }

    void operator() (Assign& o) {
      o.statementId = statementId_++;
    }
  private:
    unsigned& statementId_;
  };

  void normalizeStatementId(ProcDef& procedure, unsigned& statementId) {
    StatementIdNormalizer normalizer{statementId};
    for (auto& statement: procedure.statements) {
      std::visit(normalizer, statement);
    }
  }
  } // anonymous 

  void normalizeStatementId(Program& program, pkb::PKBTableManager& mgr) {
    unsigned statementId = 1;
    for (auto& procedure: program) {
      normalizeStatementId(procedure, statementId);
    }
    auto& namesTable = mgr.getNamesTable();
    auto& nameTypesTable  = mgr.getNameTypesTable();
    for (unsigned i = 1 ; i < statementId; ++i) {
      namesTable.insert(i, std::to_string(i));
      nameTypesTable.insert(i, EntityId::Statement);
    }
  }
}