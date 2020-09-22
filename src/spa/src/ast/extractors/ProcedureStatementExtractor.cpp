#include <ProcedureStatementExtractor.h>
#include <PKBTableManager.h>
#include <ASTToString.h>
#include <SPAAssert.h>
namespace ast {
  using namespace common;
  namespace {
  struct ProcedureStatementExtractor {
    ProcedureStatementExtractor(unsigned nameId, pkb::PKBTableManager& mgr)
    : procedureId_(nameId)
    , table_(mgr.getProcedureStatements()) { }
    void operator() (const Read& o) {
      table_.insert(procedureId_, o.statementId);
    }

    void operator() (const Print& o) {
      table_.insert(procedureId_, o.statementId);
    }

    void operator() (const Call& o) {
      table_.insert(procedureId_, o.statementId);
    }

    void operator() (const While& o) {
      table_.insert(procedureId_, o.statementId);
      for (auto& s: o.statements) {
        std::visit(*this, s);
      }
    }

    void operator() (const If& o) {
      table_.insert(procedureId_, o.statementId);
      for (auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for (auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
    }

    void operator() (const Assign& o) {
      table_.insert(procedureId_, o.statementId);
    }
  private:
    unsigned procedureId_;
    pkb::PKBTable<unsigned, unsigned>& table_; // ProcedureStatementTable
  };
  } // anonymous

  void populateProcedureStatementTable(const ProcDef& procedure, pkb::PKBTableManager& mgr) {
    auto& namesTable = mgr.getNamesTable();
    const auto& filtered = namesTable.filterRight(procedure.name);
    SPA_ASSERT(filtered.size() == 1);
    const auto nameId = *filtered.begin();
    ProcedureStatementExtractor extractor{nameId, mgr};
    for (const auto& statement: procedure.statements) {
      std::visit(extractor, statement);
    }
  }
}
