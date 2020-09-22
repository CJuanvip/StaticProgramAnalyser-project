#include <catch.hpp>
#include <ParentExtractor.h>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <vector>
namespace {
  void verify(pkb::PKBTable<unsigned, unsigned>& table, unsigned lhs, const std::vector<unsigned> rhs) {
    const auto& filtered = table.filterLeft(lhs);
    REQUIRE(filtered.size() == rhs.size());
    for (auto expected: rhs) {
      REQUIRE(filtered.find(expected) != filtered.end());
    }
  }
} //anonymous
TEST_CASE("Parent tables extracted correctly") {
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  normalizeStatementId(program, mgr);
  for(const auto& procedure: program) {
    extractParent(procedure, mgr);
  }
  auto& parentTable = mgr.getParent();
  REQUIRE(parentTable.select().size() == 7);
  auto& parentClosureTable = mgr.getParentClosure();
  REQUIRE(parentClosureTable.select().size() == 7);
  verify(parentTable, 14, {15, 16, 17, 18});
  verify(parentTable, 19, {20, 21, 22});
}