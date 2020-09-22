#include <catch.hpp>
#include <ConstructAST.h>
#include <NextExtractor.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <StatementExtractors.h>
#include <CFGExtractor.h>
#include <PKBTable.h>
#include <algorithm>
using namespace ast;
namespace {
  using namespace pkb;
  void verify(const std::vector<std::vector<unsigned>>& expected, const std::vector<PKBResult<unsigned, unsigned>>& got) {
    std::vector<std::pair<unsigned, unsigned>> flattenedExpected;
    for (size_t src = 0; src < expected.size(); ++src) {
      for (const auto dest: expected[src]) {
        if (dest != 0) {
          flattenedExpected.push_back({src, dest});
        }
      }
    }
    std::vector<std::pair<unsigned, unsigned>> flattenedGot;
    for (const auto& result: got) {
      flattenedGot.push_back({result.getLeft(), result.getRight()});
    }
    REQUIRE(flattenedExpected.size() == flattenedGot.size());
    auto order = [](const std::pair<unsigned, unsigned>& p1, std::pair<unsigned, unsigned>& p2) {
      if (p1.first > p2.first) {
        return true;
      } else if (p1.first < p2.first) {
        return false;
      } else {
        return p1.second > p2.second;
      }
    }; // > operator
    std::sort(flattenedExpected.begin(), flattenedExpected.end(), order);
    std::sort(flattenedGot.begin(), flattenedGot.end(), order);
    for (size_t i = 0; i < flattenedExpected.size(); ++i) {
      REQUIRE(flattenedExpected[i].first == flattenedGot[i].first);
      REQUIRE(flattenedExpected[i].second == flattenedGot[i].second);
    }
  }
}// namespace anonymous
TEST_CASE("Test next extractor TC1") {
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
  extractNext(graph, mgr);
  verify(graph.getAdjList(), mgr.getNext().select());
}
TEST_CASE("Test next extractor TC2") {
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
  extractNext(graph, mgr);
  verify(graph.getAdjList(), mgr.getNext().select());
}
TEST_CASE("Test next extractor TC3") {
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
  extractNext(graph, mgr);
  verify(graph.getAdjList(), mgr.getNext().select());
}
TEST_CASE("Test next extractor TC4") {
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
  extractNext(graph, mgr);
  verify(graph.getAdjList(), mgr.getNext().select());
}