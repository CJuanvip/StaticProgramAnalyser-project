#include <UseExtractor.h>
#include <UseProcessor.h>
#include <ASTToString.h>
#include <AST.h>
#include <SPAAssert.h>
namespace ast {
  using namespace common;
  struct UsesMinusCallsVisitor {
    UsesMinusCallsVisitor(unsigned procedureId,
                          std::vector<unsigned> &enclosingStatements,
                          pkb::UseProcessor& useProcessor,
                          pkb::PKBTableManager& mgr)
    : mgr_(mgr)
    , useProcessor_(useProcessor)
    , procId_(procedureId)
    , enclosingStmts_(enclosingStatements) { }

    void operator() (const While& o) {
      const auto& variablesAndConstantsUsed = mgr_.getStatementVariables().filterLeft(o.statementId);
      const auto variablesUsed = retainVariables(variablesAndConstantsUsed);
      // add the variables appearing in the conditional expression
      for (const auto& varId : variablesUsed) {
        useProcessor_.insertUseEntityVariableMinusCall(o.statementId, varId);
        useProcessor_.insertUseEntityVariableMinusCall(procId_, varId);
      }

      // add to all enclosing stmts
      for (const auto& stmtId : enclosingStmts_) {
        for (const auto& varId : variablesUsed) {
          useProcessor_.insertUseEntityVariableMinusCall(stmtId, varId);
        }
      }

      // recurse into nested stmts
      enclosingStmts_.push_back(o.statementId);
      for (const auto& statement: o.statements) {
        std::visit(*this, statement);
      }
      enclosingStmts_.pop_back();
    }

    void operator() (const If& o) {
      const auto& variablesAndConstantsUsed = mgr_.getStatementVariables().filterLeft(o.statementId);
      const auto variablesUsed = retainVariables(variablesAndConstantsUsed);
      for (const auto& varId : variablesUsed) {
        useProcessor_.insertUseEntityVariableMinusCall(o.statementId, varId);
        useProcessor_.insertUseEntityVariableMinusCall(procId_, varId);
      }

      for (const auto& stmtId : enclosingStmts_) {
        for (const auto& varId : variablesUsed) {
          useProcessor_.insertUseEntityVariableMinusCall(stmtId, varId);
        }
      }

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
      const auto& filtered = mgr_.getNamesTable().filterRight(toString(o.procedure));
      SPA_ASSERT(filtered.size() == 1);
      auto procedureId = *filtered.begin();
      useProcessor_.insertCallStatementProcedureMapping(o.statementId, procedureId);
    }

    void operator() (const Read& ) { }

    void operator() (const Print& o) {
      const auto& variablesUsed = mgr_.getStatementVariables().filterLeft(o.statementId);
      for (const auto& varId : variablesUsed) {
        useProcessor_.insertUseEntityVariableMinusCall(o.statementId, varId);
        useProcessor_.insertUseEntityVariableMinusCall(procId_, varId);
      }

      for (const auto& stmtId : enclosingStmts_) {
        for (const auto& varId : variablesUsed) {
          useProcessor_.insertUseEntityVariableMinusCall(stmtId, varId);
        }
      }
    }

    void operator() (const Assign& o) {
      const auto& namesUsed = mgr_.getAssignRhs().filterLeft(o.statementId);
      auto& nameTypesTable = mgr_.getNameTypesTable();
      std::vector<unsigned> variablesUsed;
      for (const auto& namesId: namesUsed) {
        const auto& filtered = nameTypesTable.filterLeft(namesId);
        if (filtered.find(EntityId::Constant) == filtered.end()) {
          variablesUsed.push_back(namesId);
        }
      }
      for (const auto& varId : variablesUsed) {
        useProcessor_.insertUseEntityVariableMinusCall(o.statementId, varId);
        useProcessor_.insertUseEntityVariableMinusCall(procId_, varId);
      }

      for (const auto& stmtId : enclosingStmts_) {
        for (const auto& varId : variablesUsed) {
          useProcessor_.insertUseEntityVariableMinusCall(stmtId, varId);
        }
      }
    }

  private:
    std::vector<unsigned> retainVariables(const std::unordered_set<unsigned>& set) {
      std::vector<unsigned> ret;
      const auto& nameTypesTable = mgr_.getNameTypesTable();
      for (const auto id: set) {
        const auto& filtered = nameTypesTable.filterLeft(id);
        if (filtered.find(common::EntityId::Constant) != filtered.end()) {
          continue;
        }
        ret.push_back(id);
      }
      return ret;
    }
    pkb::PKBTableManager& mgr_;
    pkb::UseProcessor& useProcessor_;
    unsigned procId_;
    std::vector<unsigned>& enclosingStmts_;
  };

  void populateUsesEntityVariableMinusCalls(const ProcDef& proc,
                                                 pkb::UseProcessor& useProcessor,
                                                 pkb::PKBTableManager& mgr) {
    std::vector<unsigned> enclosingStmts;
    // get the procId
    const auto& filtered = mgr.getNamesTable().filterRight(proc.name);
    SPA_ASSERT( filtered.size() == 1 );
    auto procId = *filtered.begin();
    UsesMinusCallsVisitor visitor(procId, enclosingStmts, useProcessor, mgr);
    auto recurseTransferUses = [&visitor](const Stmt& statement) {
      std::visit(visitor, statement);
    };

    for (const auto& statement: proc.statements) {
      recurseTransferUses(statement);
    }
  }

  void populateUsesEntityVariableTable(pkb::UseProcessor& useProcessor, pkb::PKBTableManager& mgr) {
    // we first populate all the vars used by each procedure
    const auto& procIds = mgr.getNameTypesTable().filterRight(EntityId::Procedure);
    auto& usesEntityVariableTable = mgr.getUsesEntityVariableTable();
    for (const auto& procId : procIds) {
      // add in the variables that this procedure itself uses
      const auto& varsUsedBySelf = useProcessor.getUseEntityVariableMinusCall().filterLeft(procId);
      for (const auto& varId : varsUsedBySelf) {
          usesEntityVariableTable.insert(procId, varId);
      }
      // add in the variables used by procedures that this procedure eventually calls
      const auto& callees = mgr.getCallClosure().filterLeft(procId);
      for (const auto& callee : callees) {
        const auto& varsUsedByCallee = useProcessor.getUseEntityVariableMinusCall().filterLeft(callee);
        for (const auto& varId : varsUsedByCallee) {
          usesEntityVariableTable.insert(procId, varId);
        }
      }
    }
    // copy over getUsesStatementVariableMinusCallsTable
    const auto& usesEntVarMinusCalls = useProcessor.getUseEntityVariableMinusCall().select();
    // copy the table
    for (const auto& result : usesEntVarMinusCalls) {
      usesEntityVariableTable.insert(result.getLeft(), result.getRight());
    }
    // add in <call stmt id, varId>
    const auto& callStmts = mgr.getStatementsTable().filterRight(EntityId::Call);
    for (const auto callStmtId : callStmts) {
      const auto& filtered = useProcessor.getCallStatementProcedureMapping().filterLeft(callStmtId);
      SPA_ASSERT(filtered.size() == 1);
      unsigned procId = *filtered.begin();
      // we have just populated this table, so we can use it
      const auto& varsUsedByCall = usesEntityVariableTable.filterLeft(procId);
      const auto& parents = mgr.getParentClosure().filterRight(callStmtId);
      // add (callStmtId -> vars)
      // add (parentId -> vars)
      for (const auto& varId : varsUsedByCall) {
        usesEntityVariableTable.insert(callStmtId, varId);
        for (const auto& parentId : parents) {
          usesEntityVariableTable.insert(parentId, varId);
        }
      }
    }
  }

  void extractUse(const Program& program,
                  pkb::UseProcessor& useProcessor,
                  pkb::PKBTableManager& mgr) {
    for (const auto& procedure: program) {
      populateUsesEntityVariableMinusCalls(procedure, useProcessor, mgr);
    }
    populateUsesEntityVariableTable(useProcessor, mgr);
  }
}
