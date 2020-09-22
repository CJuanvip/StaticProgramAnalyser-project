#include <catch.hpp>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <EntityId.h>
using namespace common;
struct StatementIdVerifier {
  StatementIdVerifier(unsigned& initStatmentId)
  : expectedStatementId_(initStatmentId) { }
  void operator() (const Read& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
  }

  void operator() (const Print& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
  }

  void operator() (const Call& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
  }

  void operator() (const While& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
    for (auto& s: o.statements) {
      std::visit(*this, s);
    }
  }

  void operator() (const If& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
    for (auto& s: o.thenStatements) {
      std::visit(*this, s);
    }
    for (auto& s: o.elseStatements) {
      std::visit(*this, s);
    }
  }

  void operator() (const Assign& o) {
    REQUIRE(o.statementId == expectedStatementId_++);
  }
private:
  unsigned& expectedStatementId_;
};
TEST_CASE("Statement Id normalized correctly") {
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  normalizeStatementId(program, mgr);
  // VERIFY
  unsigned nextStatementId = 1;
  StatementIdVerifier v{nextStatementId};
  for(const auto& procedure: program) {
    for(const auto& statement: procedure.statements) {
      std::visit(v, statement);
    }
  }
  auto& namesTable = mgr.getNamesTable();
  REQUIRE(namesTable.count() == nextStatementId - 1);
  auto& nameTypesTable = mgr.getNameTypesTable();
  for(unsigned i = 1; i < nextStatementId; ++i) {
    REQUIRE(*namesTable.filterLeft(i).begin() == std::to_string(i));
    REQUIRE(*nameTypesTable.filterLeft(i).begin() == EntityId::Statement);
  }
}