#include <catch.hpp>
#include <ProcedureNameExtractor.h>
#include <NameExtractor.h>
#include <ConstructAST.h>
#include <PKBTableManager.h>
#include <EntityId.h>
namespace {
using namespace common;
void verify(const std::vector<std::string>& names, pkb::PKBTableManager& mgr, EntityId type) {
  auto& namesTable = mgr.getNamesTable();
  auto& namesTypeTable = mgr.getNameTypesTable();
  for (auto& n: names) {
    const auto& filtered = namesTable.filterRight(n);
    REQUIRE(filtered.size()==1);
    auto nameId = *filtered.begin();
    const auto& nameIds = namesTypeTable.filterRight(type);
    REQUIRE(nameIds.find(nameId) != nameIds.end());
  }
}
} //anonymous
TEST_CASE("Name is extracted correctly TC1") {
  using namespace ast;
  auto program = buildTestCase();
  pkb::PKBTableManager mgr;
  NameExtractor extractor(mgr);
  // Prereq: procedure name is populated first
  for(const auto& procedure: program) {
    transferProcedureName(procedure, mgr);
  }
  for (const auto& procedure: program) {
    for(const auto& statement: procedure.statements) {
      std::visit(extractor, statement);
    }
  }
  std::vector<std::string> procedureNames{"main", "readPoint", "printResults", "computeCentroid"};
  std::vector<std::string> variableNames{"flag", "x", "y", "cenX", "cenY", "normSq", "count"};
  std::vector<std::string> constantNames{"0", "1"};
  auto& namesTable = mgr.getNamesTable();
  REQUIRE(namesTable.select().size() == procedureNames.size() + variableNames.size() + constantNames.size());
  verify(procedureNames, mgr, EntityId::Procedure);
  verify(variableNames, mgr, EntityId::Variable);
  verify(constantNames, mgr, EntityId::Constant);
}

TEST_CASE("Name is extracted correctly TC2") {
  using namespace ast;
  auto program = buildTestCase2();
  pkb::PKBTableManager mgr;
  NameExtractor extractor(mgr);
  // Prereq: procedure name is populated first
  for(const auto& procedure: program) {
    transferProcedureName(procedure, mgr);
  }
  for (const auto& procedure: program) {
    for(const auto& statement: procedure.statements) {
      std::visit(extractor, statement);
    }
  }
  std::vector<std::string> procedureNames{"procA", "procB"};
  std::vector<std::string> variableNames{"a", "c", "b"};
  std::vector<std::string> constantNames{"10", "100", "3", "20"};
  auto& namesTable = mgr.getNamesTable();
  REQUIRE(namesTable.select().size() == procedureNames.size() + variableNames.size() + constantNames.size());
  verify(procedureNames, mgr, EntityId::Procedure);
  verify(variableNames, mgr, EntityId::Variable);
  verify(constantNames, mgr, EntityId::Constant);
}