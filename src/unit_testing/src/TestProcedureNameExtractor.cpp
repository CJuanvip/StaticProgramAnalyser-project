#include <catch.hpp>
#include <ConstructAST.h>
#include <ProcedureNameExtractor.h>
#include <EntityId.h>
using namespace ast;
TEST_CASE("Procedure Name Extracted Correctly") {
  auto program = buildTestCase();
  pkb::PKBTableManager mgr;
  for(const auto& procedure: program) {
    transferProcedureName(procedure, mgr);
  }
  auto& namesTable = mgr.getNamesTable();
  auto& nameTypesTable = mgr.getNameTypesTable();
  std::vector<std::string> expected { "main", "readPoint", "printResults", "computeCentroid"};
  for(auto expect: expected) {
    const auto& actual = namesTable.filterRight(expect);
    REQUIRE(actual.size() == 1);
    REQUIRE(*nameTypesTable.filterLeft(*actual.begin()).begin() == common::EntityId::Procedure);
  }
}