#include <catch.hpp>
#include <NextClosureEvaluator.h>
#include <ConstructAST.h>
#include <NextExtractor.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <StatementExtractors.h>
#include <CFGExtractor.h>
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
TEST_CASE("Test Next Closure Evaluator TC1") {
  auto program = buildTestCase();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(cfg, program, mgr);
  extractNext(cfg, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    {2,3},
    {3},
    {},
    {5},
    {},
    {7,8,9},
    {8,9},
    {9},
    {},
    {11,12,13,14,15,16,17,18,19,20,21,22,23},
    {12,13,14,15,16,17,18,19,20,21,22,23},
    {13,14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {14,15,16,17,18,19,20,21,22,23},
    {20,21,22,23},
    {23},
    {22, 23},
    {23},
    {}
  };
  verify(ans, mgr.getNextClosure());
}

TEST_CASE("Test Next Closure Evaluator TC2") {
  auto program = buildTestCase2();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(cfg, program, mgr);
  extractNext(cfg, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    {2,3,4},
    {3,4},
    {4},
    {},
    {6,7,8,9,10,11,12,13},
    {7,8,9,10,11,12,13},
    {8},
    {},
    {9, 10, 11, 12, 13},
    {9, 10, 11, 12, 13},
    {9, 10, 11, 12, 13},
    {9, 10, 11, 12, 13},
    {9, 10, 11, 12, 13}
  };
  verify(ans, mgr.getNextClosure());
}

TEST_CASE("Test Next Closure Evaluator TC4") {
  auto program = buildTestCase4();
  PKBTableManager mgr;
  CallGraph callGraph;
  ControlFlowGraph cfg;
  callGraph.initialize(program.size());
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(cfg, program, mgr);
  extractNext(cfg, mgr);
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose
    {},
    {2,3},
    {2,3},
    {5,6},
    {},
    {},
    {7,8,9,10,11},
    {7,8,9,10,11},
    {7,8,9,10,11},
    {7,8,9,10,11},
    {7,8,9,10,11},
    {}
  };
  verify(ans, mgr.getNextClosure());
}