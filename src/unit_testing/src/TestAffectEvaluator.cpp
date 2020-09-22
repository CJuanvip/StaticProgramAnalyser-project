#include <catch.hpp>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <StatementExtractors.h>
#include <CFGExtractor.h>
#include <AffectEvaluator.h>
#include <ModifyExtractor.h>
#include <UseExtractor.h>
#include <CallExtractor.h>
using namespace ast;
using namespace pkb;
using namespace common;
namespace {
  void verify(const std::vector<std::vector<unsigned>>& expected, RealtimePKBTable<unsigned, unsigned>& table) {
    unsigned expectedSize = 0;
    for(size_t src = 0; src < expected.size(); ++src) {
      expectedSize += expected[src].size();
      for(const auto dest: expected[src]) {
        REQUIRE(table.contains(src, dest) == true);
      }
    }
    REQUIRE(expectedSize == table.count());

  } 
}
TEST_CASE("Test Affect Evaluator TC1") {
  auto program = buildTestCase();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  ModifyProcessor modifyProcessor;
  UseProcessor useProcessor;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }
  extractCall(callGraph, mgr);
  extractModify(program, modifyProcessor, callGraph, mgr);
  extractUse(program, useProcessor, mgr);
  extractCFG(cfg, program, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    
    {},
    {},
    {},
    {},
    {},
    
    {},
    {},
    {},
    {},
    {15, 21, 22},

    {16, 21, 23},
    {17, 22, 23},
    {},
    {},
    {15, 21, 22},

    {16, 21, 23},
    {17, 22, 23},
    {},
    {},
    {},

    {23},
    {23},
    {}
  };
  verify(ans, mgr.getAffect());
}

TEST_CASE("Test Affect Evaluator TC2") {
  auto program = buildTestCase2();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  ModifyProcessor modifyProcessor;
  UseProcessor useProcessor;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }
  extractCall(callGraph, mgr);
  extractModify(program, modifyProcessor, callGraph, mgr);
  extractUse(program, useProcessor, mgr);
  extractCFG(cfg, program, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    
    {},
    {},
    {},
    {},
    {10, 12, 13},
    
    {},
    {8},
    {},
    {},
    {10},
    
    {},
    {10, 12, 13},
    {10, 12, 13}
  };
  verify(ans, mgr.getAffect());
}

TEST_CASE("Test Affect Evaluator TC4") {
  auto program = buildTestCase4();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  ModifyProcessor modifyProcessor;
  UseProcessor useProcessor;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }
  extractCall(callGraph, mgr);
  extractModify(program, modifyProcessor, callGraph, mgr);
  extractUse(program, useProcessor, mgr);
  extractCFG(cfg, program, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    
    {},
    {},
    {},
    {},
    {},
    
    {},
    {},
    {},
    {},
    {},
    
    {10, 11}

  };
  verify(ans, mgr.getAffect());
}

TEST_CASE("Test Affect Evaluator TC7") {
  auto program = buildTestCase7();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  ModifyProcessor modifyProcessor;
  UseProcessor useProcessor;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }
  extractCall(callGraph, mgr);
  extractModify(program, modifyProcessor, callGraph, mgr);
  extractUse(program, useProcessor, mgr);
  extractCFG(cfg, program, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    
    {},
    {},
    {},
    {4, 6, 8, 9, 10, 11, 12, 13},
    {},
    
    {},
    {},
    {},
    {},
    {},
    
    {},
    {},
    {}

  };
  verify(ans, mgr.getAffect());
}

TEST_CASE("Test Affect Evaluator TC8") {
  auto program = buildTestCase8();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  ModifyProcessor modifyProcessor;
  UseProcessor useProcessor;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }
  extractCall(callGraph, mgr);
  extractModify(program, modifyProcessor, callGraph, mgr);
  extractUse(program, useProcessor, mgr);
  extractCFG(cfg, program, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    
    {9},
    {},
    {},
    {},
    {6},
    
    {8},
    {},
    {10},
    {5},
    {}
  };
  verify(ans, mgr.getAffect());
}