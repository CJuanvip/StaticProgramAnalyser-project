#include <catch.hpp>
#include <DesignExtractor.h>
#include <ConstructAST.h>
#include <pkb/PKBTableManager.h>
#include <query_evaluator/TableCacher.h>
#include <query_evaluator/Evaluator.h>
#include <parser/AST.h>
#include <Parser.h>
using namespace common;
// An utility class to build a dummy PKB manager
pkb::PKBTableManager setupPKBManager(Program prog = buildTestCase2()) {
  using namespace ast;
  pkb::PKBTableManager mgr;

  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(prog);
  designExtractor.extract();
  return mgr;
}

void compareResult(const std::list<std::string> &actual, const std::list<std::string> &expected) {
  REQUIRE(actual.size() == expected.size());

  std::list<std::string> actualSorted = actual;
  actualSorted.sort();
  std::list<std::string> expectedSorted = expected;
  expectedSorted.sort();
  REQUIRE(actualSorted == expectedSorted);
}

std::list<std::string> getNames(const query_eval::TableCacher &cache, const std::unordered_set<query_eval::CellType>& container) {
  std::list<std::string> result;
  for (const auto& item: container) {
    result.emplace_back(cache.getName(item));
  }
  return result;
}
std::list<std::string> getNames(const query_eval::TableCacher &cache, const query_eval::ResultTable& table) {
  std::list<std::string> result;
  std::unordered_set<query_eval::CellType> resultSet;
  for (const auto& row: table.data()) {
    REQUIRE(row.size() == 1);
    if (resultSet.count(row[0]) != 0) continue;
    resultSet.insert(row[0]);
    result.emplace_back(cache.getName(row[0]));
  }
  return result;
}

TEST_CASE("Evaluator hasSemanticError") {
  using namespace query_eval;
  using namespace common;
  pkb::PKBTableManager manager = setupPKBManager();
  TableCacher cache(manager);

  SECTION("Check result is defined") {
    Query query;
    auto declaredA = GENERATE(as<std::string>{}, "x", "y");
    auto declaredB = GENERATE(as<std::string>{}, "a", "b");
    auto selected = GENERATE(as<std::string>{}, "a", "b", "x", "y", "z");
    query.declarations.push_back({EntityId::Variable, declaredA});
    query.declarations.push_back({EntityId::Statement, declaredB});
    query.result = {(AttrRef) selected};

    bool expected = (selected != declaredA) && (selected != declaredB);
    DYNAMIC_SECTION("Test " << declaredA << "-" << declaredB << " Req: " << selected) {
      Evaluator evaluator(cache);
      REQUIRE(evaluator.hasSemanticError(query) == expected);
    }
  }
  SECTION("Check duplicate synonym") {
    Query query;
    auto declaredA = GENERATE(as<std::string>{}, "x", "y");
    auto declaredB = GENERATE(as<std::string>{}, "x", "z");
    auto selected = GENERATE(as<std::string>{}, "x", "y", "z");
    query.declarations.push_back({EntityId::Assign, declaredA});
    query.declarations.push_back({EntityId::Statement, declaredB});
    query.result = {(AttrRef) selected};

    bool expected = ((selected != declaredA) && (selected != declaredB)) || (declaredA == declaredB);
    DYNAMIC_SECTION("Test " << declaredA << "-" << declaredB << " Req: " << selected) {
      Evaluator evaluator(cache);
      REQUIRE(evaluator.hasSemanticError(query) == expected);
    }
  }
  // TODO: check relation pairs whether correct
  SECTION("Check RelationPair arguments") {
    for (auto &[relationString, relationId] : RELATION_STRING_ID) {
      for (unsigned lhsId = 0; lhsId < NUM_ENTITIES-1; ++lhsId) {
        for (unsigned rhsId = 0; rhsId < NUM_ENTITIES-1; ++rhsId) {
          EntityId lhsEntity = static_cast<EntityId>(lhsId);
          EntityId rhsEntity = static_cast<EntityId>(rhsId);
          Synonym lhs = "x";
          Synonym rhs = "y";
          Synonym selected = "x";
          Query query;
          query.declarations.push_back({lhsEntity, lhs});
          query.declarations.push_back({rhsEntity, rhs});
          StmtStmtRelation relation = {relationId, lhs, rhs};
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) selected};
          bool expected = RELATION_LHS.at(relationId).count(lhsEntity) == 0
            || RELATION_RHS.at(relationId).count(rhsEntity) == 0;
          DYNAMIC_SECTION("SemanticTest " <<relationString << "(" << TYPE_ID_STRING.at(lhsEntity) << ", " << TYPE_ID_STRING.at(rhsEntity) << ")") {
            Evaluator evaluator(cache);
            REQUIRE(evaluator.hasSemanticError(query) == expected);
          }
        }
      }
    }
  }

  // TODO: Check pattern for iteration 3
}
