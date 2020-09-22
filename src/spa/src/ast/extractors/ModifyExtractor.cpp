#include <ModifyExtractor.h>
#include <AST.h>
#include <ASTToString.h>
#include <PKBTable.h>
#include <PKBTableManager.h>
#include <SPAAssert.h>

namespace ast {
  using namespace common;
  struct ModifyExtractor {
    ModifyExtractor(pkb::ModifyProcessor& modifyProcessor,
                    pkb::PKBTableManager& mgr,
                    unsigned procedureId)
    : namesTable_(mgr.getNamesTable())
    , modifyProcessor_(modifyProcessor)
    , procedureId_(procedureId) { }
    void operator() (const While& o) {
      enclosingStatementIds_.push_back(o.statementId);
      for (const auto& s: o.statements) {
        std::visit(*this, s);
      }
      enclosingStatementIds_.pop_back();
    }
    void operator() (const If& o) {
      enclosingStatementIds_.push_back(o.statementId);
      for (const auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for (const auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
      enclosingStatementIds_.pop_back();
    }
    void operator() (const Call& o) {
      const auto& filtered = namesTable_.filterRight(toString(o.procedure));
      SPA_ASSERT(filtered.size() == 1);
      auto calleeId = *filtered.begin();
      enclosingStatementIds_.push_back(o.statementId);
      // It is equivalent to view a call in a while/if statement **AS IF** while/if calls
      modifyProcessor_.addCallingRelationship(procedureId_, calleeId, enclosingStatementIds_);
      enclosingStatementIds_.pop_back();
    }
    void operator() (const Read& o) {
      const auto& filtered = namesTable_.filterRight(toString(o.variable));
      SPA_ASSERT(filtered.size() == 1);
      auto variableId = *filtered.begin();
      enclosingStatementIds_.push_back(o.statementId);
      modifyProcessor_.addModifies(procedureId_, enclosingStatementIds_, variableId);
      enclosingStatementIds_.pop_back();
    }
    void operator() (const Print&) { }
    void operator() (const Assign& o) {
      const auto& filtered = namesTable_.filterRight(toString(o.variable));
      SPA_ASSERT(filtered.size() == 1);
      auto variableId = *filtered.begin();
      enclosingStatementIds_.push_back(o.statementId);
      modifyProcessor_.addModifies(procedureId_, enclosingStatementIds_, variableId);
      enclosingStatementIds_.pop_back();
    }
  private:
    pkb::PKBTable<unsigned, std::string>& namesTable_;
    pkb::ModifyProcessor& modifyProcessor_;
    std::vector<unsigned> enclosingStatementIds_;
    unsigned procedureId_;
  };

  unsigned getProcId(const std::string& procName, pkb::PKBTableManager& mgr) {
    auto& namesTable = mgr.getNamesTable();
    const auto& filtered = namesTable.filterRight(procName);
    SPA_ASSERT(filtered.size() == 1);
    return *filtered.begin();
  }

  void populateModifyProcessor(const ProcDef& procedure,
                      pkb::PKBTableManager& mgr,
                      pkb::ModifyProcessor& modifyProcessor) {
    ModifyExtractor extractor(modifyProcessor, mgr, getProcId(procedure.name, mgr));
    for(const auto& statement: procedure.statements) {
      std::visit(extractor, statement);
    }
  }

  void extractModify(const common::Program& program,
                     pkb::ModifyProcessor& modifyProcessor,
                     pkb::CallGraph& callGraph,
                     pkb::PKBTableManager& mgr) {
    for(const auto& procedure: program) {
      populateModifyProcessor(procedure, mgr, modifyProcessor);
    }
    modifyProcessor.postProcess(mgr, callGraph);
  }
}
