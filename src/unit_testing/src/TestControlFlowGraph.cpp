#include <catch.hpp>
#include <ControlFlowGraph.h>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <Visitor.h>
#include <algorithm>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
using namespace pkb;
using namespace common;
namespace {
  void verify(std::vector<std::vector<unsigned>> expected, std::vector<std::vector<unsigned>> got) {
    REQUIRE(expected.size() == got.size());
    for (size_t i = 0; i < expected.size(); ++i) {
      REQUIRE(expected[i].size() == got[i].size());
      std::sort(expected[i].begin(), expected[i].end());
      std::sort(got[i].begin(), got[i].end());
      for (size_t j = 0; j < expected[i].size(); ++j) {
        REQUIRE(expected[i][j] == got[i][j]);
      }
    }
  }
  void verifyBranchLinks(const std::vector<unsigned>& got, std::unordered_map<unsigned, unsigned>& calling) {
    size_t gotSize = 0;
    for (size_t i = 0; i < got.size(); i++) {
      auto branchLink = got[i];
      ++gotSize;
      REQUIRE(branchLink == calling[i]);
    }
    REQUIRE(gotSize == calling.size());
  }
  void verifyBackwardLinks(const std::vector<std::unordered_map<unsigned, unsigned>>& got, std::unordered_map<unsigned, std::unordered_map<unsigned, unsigned>>& backward) {
    size_t gotSize = 0;
    for(size_t i = 0; i < got.size(); ++i) {
      gotSize += got[i].size();
      if (backward.find(i) == backward.end()) {
        continue;
      } else {
        for (const auto& [k, v]: got[i]) {
          REQUIRE(backward[i].find(k) != backward[i].end());
          REQUIRE(backward[i][k] == got[i].at(k));
        }
      }
    }
    size_t expectedSize = 0;
    for(const auto& [k,v]: backward) {
      expectedSize += v.size();
    }
    REQUIRE(gotSize == expectedSize);
  }
}
TEST_CASE("Tests Control Flow TC1") {
  pkb::ControlFlowGraph g;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  ast::normalizeStatementId(program, mgr);
  auto stmtCount = mgr.getNamesTable().count();
  for (const auto& procedure: program) {
    ast::transferProcedureName(procedure, mgr);
    ast::populateProcedureStatementTable(procedure, mgr);
  }
  g.initialize(stmtCount);
  for(const auto& procedure: program) {
    g.process(procedure.statements);
  }
  g.resolveCallTasks(mgr);
  std::vector<std::vector<unsigned>> ans = {
    {},
    {2},
    {3},
    {0},
    {5},
    {0},
    {7},
    {8},
    {9},
    {0},
    {11},
    {12},
    {13},
    {14},
    {15, 19},
    {16},
    {17},
    {18},
    {14},
    {20, 21},
    {23},
    {22},
    {23},
    {0}
  };
  std::unordered_map<unsigned, unsigned> branchLinks = {
    {2, 10},
    {3, 6},
    {13, 4},
    {18, 4}
  };
  std::unordered_map<unsigned, std::unordered_map<unsigned, unsigned>> backwardLinks = {
    {23, {{2, 3}}},
    {9, {{3, 0}}},
    {5, {{13, 14}, {18, 14}}}
  };
  verify(g.getAdjList(), ans);
  verifyBranchLinks(g.getBranchLinks(), branchLinks);
  verifyBackwardLinks(g.getBackwardLinks(), backwardLinks);
}

TEST_CASE("Tests Control Flow TC2") {
    pkb::ControlFlowGraph g;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase2();
  ast::normalizeStatementId(program, mgr);
  auto stmtCount = mgr.getNamesTable().count();
  for (const auto& procedure: program) {
    ast::transferProcedureName(procedure, mgr);
    ast::populateProcedureStatementTable(procedure, mgr);
  }
  g.initialize(stmtCount);
  for(const auto& procedure: program) {
    g.process(procedure.statements);
  }
  g.resolveCallTasks(mgr);
  std::vector<std::vector<unsigned>> ans = {
    {},
    {2},
    {3},
    {4},
    {0},
    {6},
    {7, 9},
    {8},
    {0},
    {0, 10},
    {11},
    {12, 13},
    {9},
    {9}
  };
  std::unordered_map<unsigned, unsigned> links = {
    {3, 5}
  };
  std::unordered_map<unsigned, std::unordered_map<unsigned, unsigned>> backwardLinks = {
    {8, {{3, 4}}},
    {9, {{3, 4}}}
  };
  verify(g.getAdjList(), ans);
  verifyBranchLinks(g.getBranchLinks(), links);
  verifyBackwardLinks(g.getBackwardLinks(), backwardLinks);
}

TEST_CASE("Test Control Flow TC3") {
  pkb::ControlFlowGraph g;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase3();
  ast::normalizeStatementId(program, mgr);
  auto stmtCount = mgr.getNamesTable().count();
  for (const auto& procedure: program) {
    ast::transferProcedureName(procedure, mgr);
    ast::populateProcedureStatementTable(procedure, mgr);
  }
  g.initialize(stmtCount);
  for(const auto& procedure: program) {
    g.process(procedure.statements);
  }
  g.resolveCallTasks(mgr);
  std::vector<std::vector<unsigned>> ans = {
    {},
    {0, 2},
    {1},
    {0, 4},
    {3},
    {0, 6},
    {5}
  };
  std::unordered_map<unsigned, unsigned> branchLinks = {
    {2, 3},
    {6, 1}
  };
  std::unordered_map<unsigned, std::unordered_map<unsigned, unsigned>> backwardLinks = {
    {3, {{2, 1}}},
    {1, {{6, 5}}}
  };
  verify(g.getAdjList(), ans);
  verifyBranchLinks(g.getBranchLinks(), branchLinks);
  verifyBackwardLinks(g.getBackwardLinks(), backwardLinks);
}

TEST_CASE("Tests Control Flow TC4") {
    pkb::ControlFlowGraph g;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase4();
  ast::normalizeStatementId(program, mgr);
  auto stmtCount = mgr.getNamesTable().count();
  for (const auto& procedure: program) {
    ast::transferProcedureName(procedure, mgr);
    ast::populateProcedureStatementTable(procedure, mgr);
  }
  g.initialize(stmtCount);
  for(const auto& procedure: program) {
    g.process(procedure.statements);
  }
  g.resolveCallTasks(mgr);
  std::vector<std::vector<unsigned>> ans = {
    {},
    {0},
    {0, 3},
    {2},
    {5, 6},
    {0},
    {0},
    {0, 8},
    {9, 11},
    {7, 10},
    {9},
    {7},
    {0}
  };
  std::unordered_map<unsigned, unsigned> links = {
    {1, 4},
    {3, 4},
    {5, 7},
    {6, 12}
  };
  std::unordered_map<unsigned, std::unordered_map<unsigned, unsigned>> backwardLinks = {
    {5, {{3, 2}, {1, 0}}},
    {6, {{3, 2}, {1, 0}}},
    {7, {{5, 0}}},
    {12, {{6, 0}}}
  };
  verify(g.getAdjList(), ans);
  verifyBranchLinks(g.getBranchLinks(), links);
  verifyBackwardLinks(g.getBackwardLinks(), backwardLinks);
}