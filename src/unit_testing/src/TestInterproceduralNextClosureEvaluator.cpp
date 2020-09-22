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

TEST_CASE("Test Interprocedural Next Closure Evaluator TC1") {
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
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose

    {2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23},
    {6,7,8,9},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},

    {7,8,9},
    {8,9},
    {9},
    {},
    {3,4,5,6,7,8,9,11,12,13,14,15,16,17,18,19,20,21,22,23},

    {3,4,5,6,7,8,9,12,13,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,13,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},

    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,4,5,6,7,8,9,14,15,16,17,18,19,20,21,22,23},
    {3,6,7,8,9,20,21,22,23},
    {3,6,7,8,9,23},
    
    {3,6,7,8,9,22,23},
    {3,6,7,8,9,23},
    {3,6,7,8,9}
  };
  // mgr.getInterproceduralNextClosure().dump();
  verify(ans, mgr.getInterproceduralNextClosure());
}
TEST_CASE("Test Interprocedural Next Closure Evaluator TC2") {
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
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose

    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
    {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
    {4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
    {},
    {4, 6, 7, 8, 9, 10, 11, 12, 13},

    {4, 7, 8, 9, 10, 11, 12, 13},
    {4, 8},
    {4},
    {4, 9, 10, 11, 12, 13},
    {4, 9, 10, 11, 12, 13},

    {4, 9, 10, 11, 12, 13},
    {4, 9, 10, 11, 12, 13},
    {4, 9, 10, 11, 12, 13}
  };
  verify(ans, mgr.getInterproceduralNextClosure());
}
TEST_CASE("Test Interprocedural Next Closure Evaluator TC3") {
  auto program = buildTestCase3();
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
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose

    {1,2,3,4,5,6},
    {1,2,3,4,5,6},
    {1,2,3,4,5,6},
    {1,2,3,4,5,6},
    {1,2,3,4,5,6},

    {1,2,3,4,5,6}
  };
  verify(ans, mgr.getInterproceduralNextClosure());
}

TEST_CASE("Test Interprocedural Next Closure Evaluator TC4") {
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
  mgr.prepareForRealtimeEvaluation(cfg, callGraph);
  std::vector<std::vector<unsigned>> ans = {
    {}, // 0, for offset purpose

    {4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},

    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},

    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
    {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}
  };
  // mgr.getInterproceduralNextClosure().dump();
  verify(ans, mgr.getInterproceduralNextClosure());
}