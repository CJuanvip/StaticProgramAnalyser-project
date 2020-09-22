#include <catch.hpp>
#include <StatementIdNormalizer.h>
#include <FollowExtractor.h>
#include <ConstructAST.h>
namespace {
void verifyFollow(pkb::PKBTable<unsigned, unsigned>& followTable, const std::vector<unsigned> expected) {
  for (unsigned i = 0; i < expected.size() - 1; ++i) {
    unsigned j = i + 1;
    const auto& filtered = followTable.filterLeft(expected.at(i));
    REQUIRE(filtered.size() == 1);
    REQUIRE(*filtered.begin() == expected.at(j));
  }
}
void verifyFollowClosure(pkb::PKBTable<unsigned, unsigned>& followClosureTable, const std::vector<unsigned> expected) {
  for (unsigned i = 0; i < expected.size() - 1; ++i) {
    const auto& filtered = followClosureTable.filterLeft(expected.at(i));
    for(unsigned j = i + 1; j < expected.size(); ++j) {
      REQUIRE(filtered.find(expected.at(j)) != filtered.end());
    }
  }
}
}
TEST_CASE("Follow tables populated correctly") {
  using namespace common;
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  normalizeStatementId(program, mgr);
  for (const auto& procedure: program) {
    extractFollow(procedure, mgr);
  }
  auto& followTable = mgr.getFollow();
  REQUIRE(followTable.select().size() == 16);
  verifyFollow(followTable, {1,2,3});
  verifyFollow(followTable, {4, 5});
  verifyFollow(followTable, {6,7,8,9});
  verifyFollow(followTable, {10,11,12,13,14,19,23});
  verifyFollow(followTable, {15, 16, 17, 18});
  verifyFollow(followTable, {21, 22});
  auto& followClosureTable = mgr.getFollowClosure();
  REQUIRE(followClosureTable.select().size() == 3 + 1 + 6 + 21 + 6 + 1);
  verifyFollowClosure(followClosureTable, {1,2,3});
  verifyFollowClosure(followClosureTable, {4, 5});
  verifyFollowClosure(followClosureTable, {6,7,8,9});
  verifyFollowClosure(followClosureTable, {10,11,12,13,14,19,23});
  verifyFollowClosure(followClosureTable, {15, 16, 17, 18});
  verifyFollowClosure(followClosureTable, {21, 22});
}