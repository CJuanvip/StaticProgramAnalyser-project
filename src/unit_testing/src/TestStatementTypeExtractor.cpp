#include <catch.hpp>
#include <StatementIdNormalizer.h>
#include <StatementTypeExtractor.h>
#include <ConstructAST.h>
#include <PKBTableManager.h>
#include <EntityId.h>
TEST_CASE("Statement Type extracted correctly") {
  using namespace common;
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  StatementTypeExtractor typeExtractor{mgr.getStatementsTable()};
  // Prereq: Statement ids are correct
  normalizeStatementId(program, mgr);
  for(const auto& procedure: program) {
    for(const auto& statement: procedure.statements) {
      std::visit(typeExtractor, statement);
    }
  }
  // VERIFY
  auto results = mgr.getStatementsTable().select();
  REQUIRE(results.size() == 23);
  std::sort(results.begin(), results.end(), [] (const auto& a, const auto& b) {
    return a.getLeft() < b.getLeft();
  });
  for (unsigned i = 0; i < results.size(); ++i) {
    REQUIRE(results[i].getLeft() == i + 1);
  }
  std::vector<EntityId> typeIds{
    EntityId::Assign, EntityId::Call, EntityId::Call,
    EntityId::Read, EntityId::Read,
    EntityId::Print, EntityId::Print, EntityId::Print, EntityId::Print,
    EntityId::Assign, EntityId::Assign, EntityId::Assign, EntityId::Call, EntityId::While,
      EntityId::Assign, EntityId::Assign, EntityId::Assign, EntityId::Call,
    EntityId::If,
      EntityId::Assign,
      EntityId::Assign, EntityId::Assign,
    EntityId::Assign};
  for (unsigned i = 0; i < results.size(); ++i) {
    REQUIRE(results[i].getRight() == typeIds[i]);
  }
}