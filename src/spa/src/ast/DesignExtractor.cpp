#include <DesignExtractor.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <StatementExtractors.h>
#include <FollowExtractor.h>
#include <ParentExtractor.h>
#include <CallExtractor.h>
#include <ModifyExtractor.h>
#include <UseExtractor.h>
#include <CFGExtractor.h>
#include <NextExtractor.h>
#include <InterproceduralNextExtractor.h>
namespace ast {

  DesignExtractor::DesignExtractor(pkb::PKBTableManager& mgr)
  : mgr_(mgr) { }

  void DesignExtractor::initialize(const common::Program& program) {
    program_ = program;
    callGraph_.initialize(program_.size());
  }

  void DesignExtractor::extract() {
    // StatementId is normalized in program_
    // StatementId is inserted in namesTable, nameTypesTable
    normalizeStatementId(program_, mgr_);
    for (auto& procedure: program_) {
      // ProcedureName is inserted in namesTable, nameTypesTable
      transferProcedureName(procedure, mgr_);
      // ProcedureStatement table is populated
      populateProcedureStatementTable(procedure, mgr_);
    }
    for (auto& procedure: program_) {
      extractStatementsInformation(procedure, callGraph_, mgr_);
    }

    extractCall(callGraph_, mgr_);
    for (auto& procedure: program_) {
      extractFollow(procedure, mgr_);
      extractParent(procedure, mgr_);
    }
    extractModify(program_, modifyProcessor_, callGraph_, mgr_);
    extractUse(program_, useProcessor_, mgr_);

    extractCFG(controlFlowGraph_, program_, mgr_);
    extractNext(controlFlowGraph_, mgr_);
    extractInterproceduralNext(controlFlowGraph_, mgr_);

    // // sets up realtime evaluator
    mgr_.prepareForRealtimeEvaluation(controlFlowGraph_, callGraph_);
    
  }
  void DesignExtractor::clearCache() {
    mgr_.clearCache();
  }
}
