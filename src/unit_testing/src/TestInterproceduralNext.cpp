#include <catch.hpp>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <StatementInformationExtractor.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <CFGExtractor.h>
#include <InterproceduralNextExtractor.h>
#include <PKBTable.h>
using namespace ast;
using namespace pkb;
namespace {
  void verify(const std::vector<PKBResult<unsigned, unsigned>>& got, const std::vector<std::vector<unsigned>>& expected) {
    size_t expectedSize = 0;
    for (const auto& result: got) {
      const auto src = result.getLeft();
      const auto dest = result.getRight();
      REQUIRE(std::find(expected[src].begin(), expected[src].end(), dest) != expected[src].end());
    }
    for (const auto& v: expected) {
      expectedSize += v.size();
    }
    REQUIRE(got.size() == expectedSize);
  }
}
TEST_CASE("Test Interprocedural Next TC1") {
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  pkb::ControlFlowGraph graph;
  pkb::CallGraph callGraph;
  callGraph.initialize(program.size());
  // Prerequisite
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(graph, program, mgr);
  extractInterproceduralNext(graph, mgr);
    std::vector<std::vector<unsigned>> expected = {
    {},

    {2},
    {10},
    {6},
    {5},
    {14},
    
    {7},
    {8},
    {9},
    {},
    {11},

    {12},
    {13},
    {4},
    {15,19},
    {16},
    
    {17},
    {18},
    {4},
    {20, 21},
    {23},

    {22},
    {23},
    {3}
  };
  verify(mgr.getInterproceduralNext().select(), expected);
}
TEST_CASE("Test Interprocedural Next TC2") {
  pkb::PKBTableManager mgr;
  auto program = buildTestCase2();
  pkb::ControlFlowGraph graph;
  pkb::CallGraph callGraph;
  callGraph.initialize(program.size());
  // Prerequisite
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(graph, program, mgr);
  extractInterproceduralNext(graph, mgr);
  std::vector<std::vector<unsigned>> expected = {
    {},

    {2},
    {3},
    {5},
    {},
    {6},
    
    {7, 9},
    {8},
    {4},
    {10, 4},
    {11},

    {12, 13},
    {9},
    {9}
  };
  verify(mgr.getInterproceduralNext().select(), expected);
}
TEST_CASE("Test Interprocedural Next TC3") {
  pkb::PKBTableManager mgr;
  auto program = buildTestCase3();
  pkb::ControlFlowGraph graph;
  pkb::CallGraph callGraph;
  callGraph.initialize(program.size());
  // Prerequisite
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(graph, program, mgr);
  extractInterproceduralNext(graph, mgr);
  std::vector<std::vector<unsigned>> expected = {
    {},

    {2, 5},
    {3},
    {1, 4},
    {3},
    {6},

    {1}
  };
  verify(mgr.getInterproceduralNext().select(), expected);
}
TEST_CASE("Test Interprocedural Next TC4") {
  pkb::PKBTableManager mgr;
  auto program = buildTestCase4();
  pkb::ControlFlowGraph graph;
  pkb::CallGraph callGraph;
  callGraph.initialize(program.size());
  // Prerequisite
  normalizeStatementId(program, mgr);
  for (auto& procedure: program) {
    transferProcedureName(procedure, mgr);
    populateProcedureStatementTable(procedure, mgr);
  }
  for (auto& procedure: program) {
    extractStatementsInformation(procedure, callGraph, mgr);
  }

  extractCFG(graph, program, mgr);
  extractInterproceduralNext(graph, mgr);
  std::vector<std::vector<unsigned>> expected = {
    {},

    {4},
    {3},
    {4},
    {5, 6},
    {7},
    
    {12},
    {2, 8},
    {9,11},
    {7, 10},
    {9},

    {7},
    {2}
  };
  verify(mgr.getInterproceduralNext().select(), expected);
}