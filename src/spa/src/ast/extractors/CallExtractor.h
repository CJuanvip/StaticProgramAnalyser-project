#pragma once
#include <StatementExtractors.h>
#include <CallGraph.h>
#include <SPAAssert.h>
namespace ast {
  struct CallExtractor : public StatementVisitorBase {
    CallExtractor(pkb::PKBTableManager& mgr, pkb::CallGraph& callGraph)
    : procedureStatementTable_(mgr.getProcedureStatements())
    , namesTable_(mgr.getNamesTable())
    , callGraph_(callGraph) { }
    void operator() (const common::Read&) override { }
    void operator() (const common::Print&) override { }
    void operator() (const common::Call& o) override {
      const auto& filteredCaller = procedureStatementTable_.filterRight(o.statementId);
      SPA_ASSERT(filteredCaller.size() == 1);
      auto callerProcId = *filteredCaller.begin();

      const auto& filteredCallee = namesTable_.filterRight(toString(o.procedure));
      SPA_ASSERT(filteredCallee.size() == 1);
      auto calleeProcId = *filteredCallee.begin();
      callGraph_.addFunctionCall(callerProcId, calleeProcId);
    }

    void operator() (const common::While& o) override {
      for(const auto& s: o.statements) {
        std::visit(*this, s);
      }
    }

    void operator() (const common::If& o) override {
      for(auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for(auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
    }

    void operator() (const common::Assign&) override { }
  private:
    const pkb::PKBTable<unsigned, unsigned>& procedureStatementTable_;
    const pkb::PKBTable<unsigned, std::string>& namesTable_;
    pkb::CallGraph& callGraph_;
  };

  void extractCall(const pkb::CallGraph&, pkb::PKBTableManager&);
}
