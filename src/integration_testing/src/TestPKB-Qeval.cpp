#include <catch.hpp>
#include <ConstructAST.h>
#include <DesignExtractor.h>
#include <pkb/PKBTableManager.h>
#include <query_evaluator/utils/PKBTableManipulations.h>
#include <query_evaluator/TableCacher.h>
#include <query_evaluator/Evaluator.h>
#include <parser/AST.h>
#include <parser/Parser.h>
#include <map>
using namespace common;
// An utility class to build a dummy PKB manager
// pkb setupPKBManager(Program prog = buildTestCase2()) {
//   using namespace ast;
//   pkb::PKBTableManager mgr;

//   DesignExtractor designExtractor(mgr);
//   designExtractor.initialize(prog);
//   designExtractor.extract();
//   return mgr;
// }

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
pkb::PKBTable<unsigned, unsigned> materialize(pkb::RealtimePKBTable<unsigned, unsigned> *base) {
  pkb::PKBTable<unsigned, unsigned> result;
  for (const auto&row: base->select()) {
    result.insert(row.getLeft(), row.getRight());
  }
  return result;
}

TEST_CASE("Evaluator Select Single Entity") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;
  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);

  std::unordered_map<EntityId, std::list<std::string>> entities = {
    { EntityId::Statement, getNames(cache, manager.getStatementsTable().selectLeft()) },
    { EntityId::ProgLine, getNames(cache, manager.getStatementsTable().selectLeft()) },
    { EntityId::Read, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Read)) },
    { EntityId::Print, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Print)) },
    { EntityId::Call, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Call)) },
    { EntityId::While, getNames(cache, manager.getStatementsTable().filterRight(EntityId::While)) },
    { EntityId::If, getNames(cache, manager.getStatementsTable().filterRight(EntityId::If)) },
    { EntityId::Assign, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Assign)) },
    { EntityId::Procedure, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Procedure)) },
    { EntityId::Constant, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Constant)) },
    { EntityId::Variable, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Variable)) }
  };

  for (auto &[entityId, expected]: entities) {
    DYNAMIC_SECTION("Single " << TYPE_ID_STRING.at(entityId)) {
      Evaluator evaluator(cache);
      const std::string& resultName = TYPE_ID_STRING.at(entityId);
      Query query;
      query.declarations.push_back({entityId, resultName});
      query.result = {(AttrRef) resultName};

      auto actual = evaluator.evaluate(query);
      compareResult(actual, expected);
    }
  }
}

TEST_CASE("Evaluator Select Multiple Entity") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;
  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);

  std::unordered_map<EntityId, std::list<std::string>> entities = {
    { EntityId::Statement, getNames(cache, manager.getStatementsTable().selectLeft()) },
    { EntityId::ProgLine, getNames(cache, manager.getStatementsTable().selectLeft()) },
    { EntityId::Read, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Read)) },
    { EntityId::Print, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Print)) },
    { EntityId::Call, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Call)) },
    { EntityId::While, getNames(cache, manager.getStatementsTable().filterRight(EntityId::While)) },
    { EntityId::If, getNames(cache, manager.getStatementsTable().filterRight(EntityId::If)) },
    { EntityId::Assign, getNames(cache, manager.getStatementsTable().filterRight(EntityId::Assign)) },
    { EntityId::Procedure, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Procedure)) },
    { EntityId::Constant, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Constant)) },
    { EntityId::Variable, getNames(cache, manager.getNameTypesTable().filterRight(EntityId::Variable)) }
  };
  for (auto &[entityId, expected]: entities) {
    DYNAMIC_SECTION("Multiple query " << TYPE_ID_STRING.at(entityId)) {
      Evaluator evaluator(cache);
      const std::string& resultName = TYPE_ID_STRING.at(entityId);
      Query query;
      query.declarations.push_back({entityId, resultName});
      query.result = {(AttrRef) resultName};

      for (auto &[entityId2, expected2]: entities) {
        query.declarations.push_back({entityId2, TYPE_ID_STRING.at(entityId2) + "1"});
        query.declarations.push_back({entityId2, TYPE_ID_STRING.at(entityId2) + "2"});
      }

      auto actual = evaluator.evaluate(query);
      compareResult(actual, expected);
    }
  }
}

TEST_CASE("Evaluator StmtStmtRelation SuchThat") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  std::unordered_map<RelationId, pkb::PKBTable<unsigned, unsigned>> relationTables = {
    { RelationId::Follows, manager.getFollow() },
    { RelationId::FollowsT, manager.getFollowClosure() },
    { RelationId::Parent, manager.getParent() },
    { RelationId::ParentT, manager.getParentClosure() },
    { RelationId::Next, manager.getNext() },
    { RelationId::NextT, materialize(&manager.getNextClosure()) },
    { RelationId::Affects, materialize(&manager.getAffect()) },
    { RelationId::AffectsT, materialize(&manager.getAffectClosure()) },
    { RelationId::NextBip, manager.getInterproceduralNext() },
    { RelationId::NextBipT, materialize(&manager.getInterproceduralNextClosure()) },
    { RelationId::AffectsBip, materialize(&manager.getInterproceduralAffect()) },
  };

  for (const auto&[relationId, relationTable]: relationTables) {

    // Relation(_, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, _)") {
      StmtStmtRelation relation = {relationId, Wildcard{}, Wildcard{}};

      Query query;
      EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));
      Synonym result = "s";
      query.declarations.push_back({resultEntity, result});
      query.suchThats.push_back(relation);
      query.result = {(AttrRef) result};
      auto expected = getNames(cache, entityTables.at(resultEntity));
      if (relationTable.count() == 0) expected.clear();

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      REQUIRE(evaluator.hasSemanticError(query) == false);
      compareResult(actual, expected);
    }

    // Relation(_, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, const)") {

      auto validRhs = manager.getStatementsTable().selectLeft();

      for (auto &rhs: validRhs) {
        StmtStmtRelation relation = {relationId, Wildcard{}, Int{std::to_string(rhs)}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithRight(rhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Relation(const, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, _)") {

      auto validLhs = manager.getStatementsTable().selectLeft();

      for (auto &lhs: validLhs) {
        StmtStmtRelation relation = {relationId, Int{std::to_string(lhs)}, Wildcard{}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithLeft(lhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Relation(const, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, const)") {

      auto validLhs = manager.getStatementsTable().selectLeft();
      auto validRhs = manager.getStatementsTable().selectLeft();

      for (auto &lhs: validLhs) {
        for (auto &rhs: validRhs) {
          StmtStmtRelation relation = {relationId, Int{std::to_string(lhs)}, Int{std::to_string(rhs)}};
          Query query;
          EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

          Synonym result = "s";
          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          auto expected = getNames(cache, entityTables.at(resultEntity));
          if (relationTable.contains(lhs, rhs) == 0) expected.clear();

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(_, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        Query query;
        Synonym result = "s";
        StmtStmtRelation relation = {relationId, Wildcard{}, result};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        if (RELATION_RHS.at(relationId).count(resultEntity) == 0) {
          std::list<std::string> expected = {};
          REQUIRE(evaluator.hasSemanticError(query) == true);
          compareResult(actual, expected);
        } else {
          auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.selectRight());
          auto expected = getNames(cache, expectedTable);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(synonym, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, _)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", _)") {
        Query query;
        Synonym result = "s";
        StmtStmtRelation relation = {relationId, result, Wildcard{}};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        if (RELATION_LHS.at(relationId).count(resultEntity) == 0) {
          std::list<std::string> expected = {};
          REQUIRE(evaluator.hasSemanticError(query) == true);
          compareResult(actual, expected);
        } else {
          auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.selectLeft());
          auto expected = getNames(cache, expectedTable);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(const, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        auto validLhs = manager.getStatementsTable().selectLeft();

        for (auto &lhs:validLhs) {
          Query query;
          Synonym result = "s";
          StmtStmtRelation relation = {relationId, Int{std::to_string(lhs)}, result};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_RHS.at(relationId).count(resultEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterLeft(lhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, const)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", const)") {
        auto validRhs = manager.getStatementsTable().selectLeft();

        for (auto &rhs:validRhs) {
          Query query;
          Synonym result = "s";
          StmtStmtRelation relation = {relationId, result, Int{std::to_string(rhs)}};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(resultEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterRight(rhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, synonym)") {
      EntityId lhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      EntityId rhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(lhsEntity) << ", " << TYPE_ID_STRING.at(rhsEntity) << ")") {
        Query query;
        Synonym lhsName = "x", rhsName = "y";
        StmtStmtRelation relation = {relationId, lhsName, rhsName};

        query.declarations.push_back({lhsEntity, lhsName});
        query.declarations.push_back({rhsEntity, rhsName});
        query.suchThats.push_back(relation);

        SECTION("Select left") {
          query.result = {(AttrRef) lhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(lhsEntity) == 0
              || RELATION_RHS.at(relationId).count(rhsEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({0});
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
        SECTION("Select right") {
          query.result = {(AttrRef) rhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(lhsEntity) == 0
              || RELATION_RHS.at(relationId).count(rhsEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({1});
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }
  }
}


TEST_CASE("Evaluator StmtProcVariableRelation SuchThat") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );

  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  std::unordered_map<RelationId, pkb::PKBTable<unsigned, unsigned>&> relationTables = {
    { RelationId::Modifies, manager.getModifyEntity() },
    { RelationId::Uses, manager.getUsesEntityVariableTable() },
  };

  for (const auto&[relationId, relationTable]: relationTables) {


    // Relation(_, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, _)") {

      StmtEntEntRelation relation = {relationId, (StmtRef) Wildcard{}, (EntRef) Wildcard{}};

      Query query;
      EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));
      Synonym result = "s";
      query.declarations.push_back({resultEntity, result});
      query.suchThats.push_back(relation);
      query.result = {(AttrRef) result};

      std::list<std::string> expected = {};
      Evaluator evaluator(cache);
      REQUIRE(evaluator.hasSemanticError(query) == true);
      auto actual = evaluator.evaluate(query);
      compareResult(actual, expected);
    }

    // Relation(_, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, const)") {

      auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Variable);

      for (auto &rhs: validRhs) {
        StmtEntEntRelation relation = {relationId, (EntRef) Wildcard{}, Var{cache.getName(rhs)}};

        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "x";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        std::list<std::string> expected = {};
        Evaluator evaluator(cache);
        REQUIRE(evaluator.hasSemanticError(query) == true);
        auto actual = evaluator.evaluate(query);
        compareResult(actual, expected);
      }
    }

    // Relation(const, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, _)") {

      auto validLhsEnt = manager.getNameTypesTable().filterRight(EntityId::Procedure);

      for (auto &lhs: validLhsEnt) {
        StmtEntEntRelation relation = {relationId, Var{cache.getName(lhs)}, Wildcard{}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithLeft(lhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }

      auto validLhsStmt = manager.getStatementsTable().selectLeft();
      for (auto &lhs: validLhsStmt) {
        StmtEntEntRelation relation = {relationId, Int{std::to_string(lhs)}, Wildcard{}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithLeft(lhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Relation(const, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, const)") {

      auto validLhsEnt = manager.getNameTypesTable().filterRight(EntityId::Procedure);
      auto validLhsStmt = manager.getStatementsTable().selectLeft();
      auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Variable);

      for (auto &lhs: validLhsEnt) {
        for (auto &rhs: validRhs) {
          StmtEntEntRelation relation = {relationId, Var{cache.getName(lhs)}, Var{cache.getName(rhs)}};
          Query query;
          EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

          Synonym result = "s";
          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          auto expected = getNames(cache, entityTables.at(resultEntity));
          if (relationTable.contains(lhs, rhs) == 0) expected.clear();

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }

      for (auto &lhs: validLhsStmt) {
        for (auto &rhs: validRhs) {
          StmtEntEntRelation relation = {relationId, Int{std::to_string(lhs)}, Var{cache.getName(rhs)}};
          Query query;
          EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

          Synonym result = "s";
          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          auto expected = getNames(cache, entityTables.at(resultEntity));
          if (relationTable.contains(lhs, rhs) == 0) expected.clear();

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(_, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        Query query;
        Synonym result = "s";
        StmtEntEntRelation relation = {relationId, StmtRef{Wildcard{}}, result};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      }
    }

    // Relation(synonym, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, _)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", _)") {
        Query query;
        Synonym result = "s";
        StmtEntEntRelation relation = {relationId, StmtRef{Synonym{result}}, Wildcard{}};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        if (RELATION_LHS.at(relationId).count(resultEntity) == 0) {
          std::list<std::string> expected = {};
          REQUIRE(evaluator.hasSemanticError(query) == true);
          compareResult(actual, expected);
        } else {
          ResultTable rightEntityTable = entityTables.at(EntityId::Variable).reschema({1});
          auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.select()) * rightEntityTable;
          expectedTable = expectedTable.project({0});
          auto expected = getNames(cache, expectedTable);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(const, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        auto validLhsEnt = manager.getNameTypesTable().filterRight(EntityId::Procedure);
        auto validLhsStmt = manager.getStatementsTable().selectLeft();

        for (auto &lhs:validLhsEnt) {
          Query query;
          Synonym result = "s";
          StmtEntEntRelation relation = {relationId, Var{cache.getName(lhs)}, result};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_RHS.at(relationId).count(resultEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterLeft(lhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }

        for (auto &lhs:validLhsStmt) {
          Query query;
          Synonym result = "s";
          StmtEntEntRelation relation = {relationId, Int{std::to_string(lhs)}, result};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_RHS.at(relationId).count(resultEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterLeft(lhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, const)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", const)") {
        auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Variable);

        for (auto &rhs:validRhs) {
          Query query;
          Synonym result = "s";
          StmtEntEntRelation relation = {relationId, StmtRef{result}, Var{cache.getName(rhs)}};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(resultEntity) == 0) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterRight(rhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, synonym)") {
      EntityId lhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      EntityId rhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(lhsEntity) << ", " << TYPE_ID_STRING.at(rhsEntity) << ")") {
        Query query;
        Synonym lhsName = "x", rhsName = "y";
        StmtEntEntRelation relation = {relationId, StmtRef{lhsName}, rhsName};

        query.declarations.push_back({lhsEntity, lhsName});
        query.declarations.push_back({rhsEntity, rhsName});
        query.suchThats.push_back(relation);

        SECTION("Select left") {
          query.result = {(AttrRef) lhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(lhsEntity) == 0
            || RELATION_RHS.at(relationId).count(rhsEntity) == 0 ) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({0});
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
        SECTION("Select right") {
          query.result = {(AttrRef) rhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (RELATION_LHS.at(relationId).count(lhsEntity) == 0
            || RELATION_RHS.at(relationId).count(rhsEntity) == 0 ) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query));
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({1});
            auto expected = getNames(cache, expectedTable);
            compareResult(actual, expected);
          }
        }
      }
    }
  }
}

TEST_CASE("Evaluator ProcProcRelation SuchThat") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );

  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  std::unordered_map<RelationId, pkb::PKBTable<unsigned, unsigned>&> relationTables = {
    { RelationId::Calls, manager.getCall() },
    { RelationId::CallsT, manager.getCallClosure() }
  };

  for (const auto&[relationId, relationTable]: relationTables) {

    // Relation(_, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, _)") {
      EntEntRelation relation = {relationId, Wildcard{}, Wildcard{}};

      Query query;
      EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));
      Synonym result = "s";
      query.declarations.push_back({resultEntity, result});
      query.suchThats.push_back(relation);
      query.result = {(AttrRef) result};
      auto expected = getNames(cache, entityTables.at(resultEntity));
      if (relationTable.count() == 0) expected.clear();

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      REQUIRE(evaluator.hasSemanticError(query) == false);
      compareResult(actual, expected);
    }

    // Relation(_, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, const)") {

      auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);

      for (auto &rhs: validRhs) {
        EntEntRelation relation = {relationId, Wildcard{}, Var{cache.getName(rhs)}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithRight(rhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Relation(const, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, _)") {

      auto validLhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);

      for (auto &lhs: validLhs) {
        EntEntRelation relation = {relationId, Var{cache.getName(lhs)}, Wildcard{}};
        Query query;
        EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

        Synonym result = "s";
        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (relationTable.countWithLeft(lhs) == 0) expected.clear();

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Relation(const, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, const)") {

      auto validLhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);
      auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);

      for (auto &lhs: validLhs) {
        for (auto &rhs: validRhs) {
          EntEntRelation relation = {relationId, Var{cache.getName(lhs)}, Var{cache.getName(rhs)}};
          Query query;
          EntityId resultEntity = static_cast<EntityId>(GENERATE(take(1, random(0, ((int) NUM_ENTITIES)-2))));

          Synonym result = "s";
          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          auto expected = getNames(cache, entityTables.at(resultEntity));
          if (relationTable.contains(lhs, rhs) == 0) expected.clear();

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(_, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(_, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        Query query;
        Synonym result = "s";
        EntEntRelation relation = {relationId, Wildcard{}, result};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        if (resultEntity != EntityId::Procedure) {
          std::list<std::string> expected = {};
          REQUIRE(evaluator.hasSemanticError(query) == true);
          compareResult(actual, expected);
        } else {
          auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.selectRight());
          auto expected = getNames(cache, expectedTable);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(synonym, _)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, _)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", _)") {
        Query query;
        Synonym result = "s";
        EntEntRelation relation = {relationId, result, Wildcard{}};

        query.declarations.push_back({resultEntity, result});
        query.suchThats.push_back(relation);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        if (resultEntity != EntityId::Procedure) {
          std::list<std::string> expected = {};
          REQUIRE(evaluator.hasSemanticError(query) == true);
          compareResult(actual, expected);
        } else {
          auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.selectLeft());
          auto expected = getNames(cache, expectedTable);
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
    }

    // Relation(const, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, synonym)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(const, " << TYPE_ID_STRING.at(resultEntity) << ")") {
        auto validLhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);

        for (auto &lhs:validLhs) {
          Query query;
          Synonym result = "s";
          EntEntRelation relation = {relationId, Var{cache.getName(lhs)}, result};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (resultEntity != EntityId::Procedure) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterLeft(lhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, const)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, const)") {
      EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(resultEntity) << ", const)") {
        auto validRhs = manager.getNameTypesTable().filterRight(EntityId::Procedure);

        for (auto &rhs:validRhs) {
          Query query;
          Synonym result = "s";
          EntEntRelation relation = {relationId, result, Var{cache.getName(rhs)}};

          query.declarations.push_back({resultEntity, result});
          query.suchThats.push_back(relation);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (resultEntity != EntityId::Procedure) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expectedTable = entityTables.at(resultEntity) * ResultTable(relationTable.filterRight(rhs));
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    // Relation(synonym, synonym)
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(synonym, synonym)") {
      EntityId lhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      EntityId rhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
      DYNAMIC_SECTION("" << RELATION_ID_STRING.at(relationId) << "(" << TYPE_ID_STRING.at(lhsEntity) << ", " << TYPE_ID_STRING.at(rhsEntity) << ")") {
        Query query;
        Synonym lhsName = "x", rhsName = "y";
        EntEntRelation relation = {relationId, lhsName, rhsName};

        query.declarations.push_back({lhsEntity, lhsName});
        query.declarations.push_back({rhsEntity, rhsName});
        query.suchThats.push_back(relation);

        SECTION("Select left") {
          query.result = {(AttrRef) lhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (lhsEntity != EntityId::Procedure || rhsEntity != EntityId::Procedure) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({0});
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
        SECTION("Select right") {
          query.result = {(AttrRef) rhsName};
          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (lhsEntity != EntityId::Procedure || rhsEntity != EntityId::Procedure) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable partialRight = entityTables.at(rhsEntity).reschema({1});
            auto expectedTable = entityTables.at(lhsEntity) * ResultTable(relationTable.select()) * partialRight;
            expectedTable = expectedTable.project({1});
            auto expected = getNames(cache, expectedTable);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }
  }
}

TEST_CASE("Evaluator Assign Pattern") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  ResultTable whileConditionVarTable = manager.getStatementVariables().select();
  whileConditionVarTable = whileConditionVarTable * entityTables[EntityId::While] * entityTables.at(EntityId::Variable).reschema({1});

  ResultTable ifConditionVarTable = manager.getStatementVariables().select();
  ifConditionVarTable = ifConditionVarTable * entityTables[EntityId::If] * entityTables[EntityId::Variable].reschema({1});

  auto& assignLhsVarTable = manager.getAssignLhs();
  auto& assignRhsVarTable = manager.getAssignRhs();

  // pattern a(_, _)
  SECTION("pattern a(_, _)") {
    EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    DYNAMIC_SECTION("pattern " << TYPE_ID_STRING.at(resultEntity) << "(_, _) query a") {
      Query query;
      Synonym result = "a";
      BinaryPattern pattern = {result, Wildcard{}, Wildcard{}};
      query.declarations.push_back({resultEntity, result});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (resultEntity != EntityId::Assign && resultEntity != EntityId::While) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultEntity == EntityId::While && whileConditionVarTable.size() == 0)
          expected.clear();
        if (resultEntity == EntityId::If && ifConditionVarTable.size() == 0)
          expected.clear();
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    DYNAMIC_SECTION("pattern a(_, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      Query query;
      Synonym assign = "a";
      Synonym result = "x";
      BinaryPattern pattern = {assign, Wildcard{}, Wildcard{}};
      query.declarations.push_back({resultEntity, result});
      query.declarations.push_back({EntityId::Assign, assign});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      auto expected = getNames(cache, entityTables.at(resultEntity));
      if (entityTables.at(EntityId::Assign).size() == 0) expected.clear();

      REQUIRE(evaluator.hasSemanticError(query) == false);
      compareResult(actual, expected);
    }
  }

  // pattern a(synonym, _)
  SECTION("pattern a(synonym, _)") {
    EntityId synonymEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    // Query a
    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query a") {
      Query query;
      Synonym assign = "a";
      Synonym lhs = "lhs";
      BinaryPattern pattern = {assign, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::Assign, assign});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) assign};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::Assign) * ResultTable(assignLhsVarTable.select()) * synTable;
        result = result.project({0});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    // Query lhs
    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query lhs") {
      Query query;
      Synonym assign = "a";
      Synonym lhs = "lhs";
      BinaryPattern pattern = {assign, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::Assign, assign});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) lhs};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::Assign) * ResultTable(assignLhsVarTable.select()) * synTable;
        result = result.project({1});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Query external
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query " << TYPE_ID_STRING.at(resultEntity)) {

      Query query;
      Synonym assign = "a", lhs = "lhs", result = "x";
      BinaryPattern pattern = {assign, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::Assign, assign});
      query.declarations.push_back({synonymEntity, lhs});
      query.declarations.push_back({resultEntity, result});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable resultTable = entityTables.at(EntityId::Assign) * ResultTable(assignLhsVarTable.select()) * synTable;
        resultTable = resultTable.project({1});
        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }

  // pattern a(const, _)
  SECTION("pattern a(const, _)") {
    SECTION("pattern a(const, _) query a") {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym assign = "a";
        BinaryPattern pattern = {assign, Var{cache.getName(lhs)}, Wildcard{}};
        query.declarations.push_back({EntityId::Assign, assign});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) assign};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        auto expected = getNames(cache, assignLhsVarTable.filterRight(lhs));
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern a(const, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym assign = "a", result = "x";
        BinaryPattern pattern = {assign, Var{cache.getName(lhs)}, Wildcard{}};
        query.declarations.push_back({EntityId::Assign, assign});
        query.declarations.push_back({resultEntity, result});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        ResultTable resultTable = assignLhsVarTable.filterRight(lhs);
        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }

  // pattern a(_, _factor_) , factor = var/const
  SECTION("pattern a(_, _factor_)") {
    SECTION("pattern a(_, _factor_) query a") {
      auto validRhs = manager.getNameTypesTable().selectLeft();
      for (auto &rhs: validRhs) {
        for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
          Query query;
          Synonym assign = "a";
          BinaryPattern pattern = {assign, Wildcard{}, rhsMatch};
          query.declarations.push_back({EntityId::Assign, assign});
          query.patterns.push_back(pattern);
          query.result = {(AttrRef) assign};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          auto expected = getNames(cache, assignRhsVarTable.filterRight(rhs));
          REQUIRE(evaluator.hasSemanticError(query) == false);
          compareResult(actual, expected);
        }
      }
      EntityId resultEntity = EntityId::Statement;
      DYNAMIC_SECTION("pattern a(_, _factor_) query " << TYPE_ID_STRING.at(resultEntity)) {
        auto validRhs = manager.getNameTypesTable().selectLeft();
        for (auto &rhs: validRhs) {
          for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
            Query query;
            Synonym assign = "a", result = "x";
            BinaryPattern pattern = {assign, Wildcard{}, rhsMatch};
            query.declarations.push_back({EntityId::Assign, assign});
            query.declarations.push_back({resultEntity, result});
            query.patterns.push_back(pattern);
            query.result = {(AttrRef) result};

            Evaluator evaluator(cache);
            auto actual = evaluator.evaluate(query);
            ResultTable resultTable = assignRhsVarTable.filterRight(rhs);
            auto expected = getNames(cache, entityTables.at(resultEntity));
            if (resultTable.size() == 0) expected.clear();
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }
  }

  // pattern a(synonym, _factor_) , factor = var/const
  SECTION("pattern a(synonym, _factor_)") {
    EntityId lhsEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    auto validRhs = manager.getNameTypesTable().selectLeft();

    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(lhsEntity) << ", _factor_) query a") {
      for (auto &rhs: validRhs) {
        for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
          Query query;
          Synonym assign = "a";
          Synonym lhs = "x";
          BinaryPattern pattern = {assign, lhs, rhsMatch};
          query.declarations.push_back({EntityId::Assign, assign});
          query.declarations.push_back({lhsEntity, lhs});
          query.patterns.push_back(pattern);
          query.result = {(AttrRef) assign};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (lhsEntity != EntityId::Variable) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            auto expected = getNames(cache, assignRhsVarTable.filterRight(rhs));
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(lhsEntity) << ", _factor_) query " << TYPE_ID_STRING.at(lhsEntity)) {
      for (auto &rhs: validRhs) {
        for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
          Query query;
          Synonym assign = "a";
          Synonym lhs = "x";
          BinaryPattern pattern = {assign, lhs, rhsMatch};
          query.declarations.push_back({EntityId::Assign, assign});
          query.declarations.push_back({lhsEntity, lhs});
          query.patterns.push_back(pattern);
          query.result = {(AttrRef) lhs};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (lhsEntity != EntityId::Variable) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable result = assignLhsVarTable.select();
            result = result * ResultTable(assignRhsVarTable.filterRight(rhs));
            result = result.project({1});
            auto expected = getNames(cache, result);
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern a(" << TYPE_ID_STRING.at(lhsEntity) << ", _factor_) query " << TYPE_ID_STRING.at(resultEntity)) {
      for (auto &rhs: validRhs) {
        for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
          Query query;
          Synonym assign = "a";
          Synonym lhs = "x";
          Synonym result = "r";
          BinaryPattern pattern = {assign, lhs, rhsMatch};
          query.declarations.push_back({EntityId::Assign, assign});
          query.declarations.push_back({lhsEntity, lhs});
          query.declarations.push_back({resultEntity, result});
          query.patterns.push_back(pattern);
          query.result = {(AttrRef) result};

          Evaluator evaluator(cache);
          auto actual = evaluator.evaluate(query);
          if (lhsEntity != EntityId::Variable) {
            std::list<std::string> expected = {};
            REQUIRE(evaluator.hasSemanticError(query) == true);
            compareResult(actual, expected);
          } else {
            ResultTable resultTable = assignLhsVarTable.select();
            resultTable = resultTable * ResultTable(assignRhsVarTable.filterRight(rhs));
            resultTable = resultTable.project({1});
            auto expected = getNames(cache, entityTables.at(resultEntity));
            if (resultTable.size() == 0) expected.clear();
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }
  }
  // pattern a(var, _factor_) , factor = var/const
  SECTION("pattern a(var, _factor_)") {
    auto validLhs = manager.getNameTypesTable().selectLeft();
    auto validRhs = manager.getNameTypesTable().selectLeft();
    SECTION("pattern a(var, _factor_) query a") {
      for (auto &lhs: validLhs) {
        for (auto &rhs: validRhs) {
          for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
            Query query;
            Synonym assign = "a";
            BinaryPattern pattern = {assign, Var{cache.getName(lhs)}, rhsMatch};
            query.declarations.push_back({EntityId::Assign, assign});
            query.patterns.push_back(pattern);
            query.result = {(AttrRef) assign};

            Evaluator evaluator(cache);
            auto actual = evaluator.evaluate(query);
            ResultTable result = ResultTable(assignLhsVarTable.filterRight(lhs)) * ResultTable(assignRhsVarTable.filterRight(rhs));
            auto expected = getNames(cache, result);
            if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
              expected.clear();
            }
            if (!manager.getNameTypesTable().contains(rhs, EntityId::Variable) && !manager.getNameTypesTable().contains(rhs, EntityId::Constant)) {
              expected.clear();
            }
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }

    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern a(var, _factor_) query " << TYPE_ID_STRING.at(resultEntity)) {
      for (auto &lhs: validLhs) {
        for (auto &rhs: validRhs) {
          for (auto rhsMatch: {PartialMatch{Var{cache.getName(rhs)}}, PartialMatch{Const{cache.getName(rhs)}}}) {
            Query query;
            Synonym assign = "a", result = "x";
            BinaryPattern pattern = {assign, Var{cache.getName(lhs)}, rhsMatch};
            query.declarations.push_back({EntityId::Assign, assign});
            query.declarations.push_back({resultEntity, result});
            query.patterns.push_back(pattern);
            query.result = {(AttrRef) result};

            Evaluator evaluator(cache);
            auto actual = evaluator.evaluate(query);
            ResultTable resultTable = ResultTable(assignLhsVarTable.filterRight(lhs)) * ResultTable(assignRhsVarTable.filterRight(rhs));
            auto expected = getNames(cache, entityTables.at(resultEntity));
            if (resultTable.size() == 0) expected.clear();
            if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
              expected.clear();
            }
            if (!manager.getNameTypesTable().contains(rhs, EntityId::Variable) && !manager.getNameTypesTable().contains(rhs, EntityId::Constant)) {
              expected.clear();
            }
            REQUIRE(evaluator.hasSemanticError(query) == false);
            compareResult(actual, expected);
          }
        }
      }
    }
  }
}

TEST_CASE("Evaluator While Pattern") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;
  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  ResultTable whileConditionVarTable = manager.getStatementVariables().select();
  whileConditionVarTable = whileConditionVarTable * entityTables[EntityId::While] * entityTables.at(EntityId::Variable).reschema({1});

  ResultTable ifConditionVarTable = manager.getStatementVariables().select();
  ifConditionVarTable = ifConditionVarTable * entityTables[EntityId::If] * entityTables[EntityId::Variable].reschema({1});

  // pattern w(_, _)
  SECTION("pattern w(_, _)") {
    EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    DYNAMIC_SECTION("pattern w(_, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      Query query;
      Synonym w = "w";
      Synonym result = "x";
      BinaryPattern pattern = {w, Wildcard{}, Wildcard{}};
      query.declarations.push_back({resultEntity, result});
      query.declarations.push_back({EntityId::While, w});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      auto expected = getNames(cache, entityTables.at(resultEntity));
      if (whileConditionVarTable.size() == 0) expected.clear();

      REQUIRE(evaluator.hasSemanticError(query) == false);
      compareResult(actual, expected);
    }
  }

  // pattern w(synonym, _)
  SECTION("pattern w(synonym, _)") {
    EntityId synonymEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    // Query i
    DYNAMIC_SECTION("pattern w(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query w") {
      Query query;
      Synonym w = "w";
      Synonym lhs = "lhs";
      BinaryPattern pattern = {w, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::While, w});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) w};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::While) * whileConditionVarTable * synTable;
        result = result.project({0});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    // Query lhs
    DYNAMIC_SECTION("pattern w(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query lhs") {
      Query query;
      Synonym w = "w";
      Synonym lhs = "lhs";
      BinaryPattern pattern = {w, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::While, w});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) lhs};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::While) * whileConditionVarTable * synTable;
        result = result.project({1});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Query external
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern w(" << TYPE_ID_STRING.at(synonymEntity) << ", _) query " << TYPE_ID_STRING.at(resultEntity)) {

      Query query;
      Synonym w = "w", lhs = "lhs", result = "x";
      BinaryPattern pattern = {w, lhs, Wildcard{}};
      query.declarations.push_back({EntityId::While, w});
      query.declarations.push_back({synonymEntity, lhs});
      query.declarations.push_back({resultEntity, result});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable resultTable = entityTables.at(EntityId::While) * whileConditionVarTable * synTable;
        resultTable = resultTable.project({1});
        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }

  // pattern w(const, _)
  SECTION("pattern w(const, _)") {
    SECTION("pattern w(const, _) query w") {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym w = "w";
        BinaryPattern pattern = {w, Var{cache.getName(lhs)}, Wildcard{}};
        query.declarations.push_back({EntityId::While, w});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) w};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        ResultTable resultTable = whileConditionVarTable.filter(lhs, 1);
        resultTable = resultTable.project({0});
        auto expected = getNames(cache, resultTable);
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        if (actual.size() != expected.size()) {
          printf("diff\n");
          if (expected.size() > 0) {
            printf("Expected: %s %s\n", expected.front().c_str(), cache.getName(lhs).c_str());
          }
        }
        compareResult(actual, expected);
      }
    }
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern w(const, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym w = "w", result = "x";
        BinaryPattern pattern = {w, Var{cache.getName(lhs)}, Wildcard{}};
        query.declarations.push_back({EntityId::While, w});
        query.declarations.push_back({resultEntity, result});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        ResultTable resultTable = whileConditionVarTable.filter(lhs, 1);

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }
}

TEST_CASE("Evaluator If Pattern") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  ResultTable ifConditionVarTable = manager.getStatementVariables().select();
  ifConditionVarTable = ifConditionVarTable * entityTables[EntityId::If] * entityTables[EntityId::Variable].reschema({1});

  // pattern i(_, _, _)
  SECTION("pattern i(_, _, _)") {
    EntityId resultEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    DYNAMIC_SECTION("pattern i(_, _, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      Query query;
      Synonym ifs = "i";
      Synonym result = "x";
      TernaryPattern pattern = {ifs, Wildcard{}};
      query.declarations.push_back({resultEntity, result});
      query.declarations.push_back({EntityId::If, ifs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      auto expected = getNames(cache, entityTables.at(resultEntity));
      if (ifConditionVarTable.size() == 0) expected.clear();

      REQUIRE(evaluator.hasSemanticError(query) == false);
      compareResult(actual, expected);
    }
  }

  // pattern i(synonym, _, _)
  SECTION("pattern i(synonym, _, _)") {
    EntityId synonymEntity = static_cast<EntityId>(GENERATE(range(0, ((int) NUM_ENTITIES)-2)));
    // Query i
    DYNAMIC_SECTION("pattern i(" << TYPE_ID_STRING.at(synonymEntity) << ", _, _) query w") {
      Query query;
      Synonym ifs = "i";
      Synonym lhs = "lhs";
      TernaryPattern pattern = {ifs, lhs};
      query.declarations.push_back({EntityId::If, ifs});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) ifs};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::If) * ifConditionVarTable * synTable;
        result = result.project({0});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    // Query lhs
    DYNAMIC_SECTION("pattern i(" << TYPE_ID_STRING.at(synonymEntity) << ", _, _) query lhs") {
      Query query;
      Synonym ifs = "i";
      Synonym lhs = "lhs";
      TernaryPattern pattern = {ifs, lhs};
      query.declarations.push_back({EntityId::If, ifs});
      query.declarations.push_back({synonymEntity, lhs});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) lhs};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable result = entityTables.at(EntityId::If) * ifConditionVarTable * synTable;
        result = result.project({1});
        auto expected = getNames(cache, result);
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }

    // Query external
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern i(" << TYPE_ID_STRING.at(synonymEntity) << ", _, _) query " << TYPE_ID_STRING.at(resultEntity)) {

      Query query;
      Synonym ifs = "i", lhs = "lhs", result = "x";
      TernaryPattern pattern = {ifs, lhs};
      query.declarations.push_back({EntityId::If, ifs});
      query.declarations.push_back({synonymEntity, lhs});
      query.declarations.push_back({resultEntity, result});
      query.patterns.push_back(pattern);
      query.result = {(AttrRef) result};

      Evaluator evaluator(cache);
      auto actual = evaluator.evaluate(query);
      if (synonymEntity != EntityId::Variable) {
        std::list<std::string> expected = {};
        REQUIRE(evaluator.hasSemanticError(query) == true);
        compareResult(actual, expected);
      } else {
        ResultTable synTable = entityTables.at(synonymEntity).reschema({1});
        ResultTable resultTable = entityTables.at(EntityId::If) * ifConditionVarTable * synTable;
        resultTable = resultTable.project({1});
        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }

  // pattern i(const, _, _)
  SECTION("pattern i(const, _, _)") {
    SECTION("pattern i(const, _, _) query i") {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym ifs = "i";
        TernaryPattern pattern = {ifs, Var{cache.getName(lhs)}};
        query.declarations.push_back({EntityId::If, ifs});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) ifs};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        ResultTable resultTable = ifConditionVarTable.filter(lhs, 1);
        resultTable = resultTable.project({0});
        auto expected = getNames(cache, resultTable);
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
    EntityId resultEntity = EntityId::Statement;
    DYNAMIC_SECTION("pattern i(const, _, _) query " << TYPE_ID_STRING.at(resultEntity)) {
      auto validLhs = manager.getNameTypesTable().selectLeft();
      for (auto &lhs: validLhs) {
        Query query;
        Synonym ifs = "i", result = "x";
        TernaryPattern pattern = {ifs, Var{cache.getName(lhs)}};
        query.declarations.push_back({EntityId::If, ifs});
        query.declarations.push_back({resultEntity, result});
        query.patterns.push_back(pattern);
        query.result = {(AttrRef) result};

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        ResultTable resultTable = ifConditionVarTable.filter(lhs, 1);

        auto expected = getNames(cache, entityTables.at(resultEntity));
        if (resultTable.size() == 0) expected.clear();
        if (!manager.getNameTypesTable().contains(lhs, EntityId::Variable)) {
          expected.clear();
        }
        REQUIRE(evaluator.hasSemanticError(query) == false);
        compareResult(actual, expected);
      }
    }
  }
}



TEST_CASE("Evaluator with clauses") {
  using namespace query_eval;
  using namespace common;
  using namespace ast;

  Program prog = GENERATE(
    buildTestCase(),
    buildTestCase2(),
    buildTestCase3(),
    buildTestCase4(),
    buildTestCase5(),
    buildTestCase6(),
    buildTestCase7(),
    buildTestCase8()
  );
  pkb::PKBTableManager manager;

  DesignExtractor designExtractor(manager);
  designExtractor.initialize(prog);
  designExtractor.extract();

  TableCacher cache(manager);
  std::unordered_map<EntityId, ResultTable> entityTables = {
    { EntityId::Statement, manager.getStatementsTable().selectLeft() },
    { EntityId::ProgLine, manager.getStatementsTable().selectLeft() },
    { EntityId::Read, manager.getStatementsTable().filterRight(EntityId::Read) },
    { EntityId::Print, manager.getStatementsTable().filterRight(EntityId::Print) },
    { EntityId::Call, manager.getStatementsTable().filterRight(EntityId::Call) },
    { EntityId::While, manager.getStatementsTable().filterRight(EntityId::While) },
    { EntityId::If, manager.getStatementsTable().filterRight(EntityId::If) },
    { EntityId::Assign, manager.getStatementsTable().filterRight(EntityId::Assign) },
    { EntityId::Procedure, manager.getNameTypesTable().filterRight(EntityId::Procedure) },
    { EntityId::Constant, manager.getNameTypesTable().filterRight(EntityId::Constant) },
    { EntityId::Variable, manager.getNameTypesTable().filterRight(EntityId::Variable) }
  };

  std::map<std::pair<EntityId, std::optional<AttrNameId>>, ResultTable> attrRefTables;
  for (const auto&[entityId, table]: entityTables) {
    if (TYPE_ID_ATTRIBUTE_EQUAL.count(entityId) == 0) continue;
    if (TYPE_ID_ATTRIBUTE_EQUAL.at(entityId).size() == 0) continue;
    attrRefTables[{entityId, *TYPE_ID_ATTRIBUTE_EQUAL.at(entityId).begin()}]
     = table;
  }
  attrRefTables[{EntityId::ProgLine, {}}] = entityTables[EntityId::ProgLine];
  attrRefTables[{EntityId::Read, AttrNameId::VarName}] =
    filterPKBLeftRight(
      manager.getStatementVariables(),
      manager.getStatementsTable().filterRight(EntityId::Read),
      manager.getNameTypesTable().filterRight(EntityId::Variable)
    ).select();
  attrRefTables[{EntityId::Print, AttrNameId::VarName}] =
    filterPKBLeftRight(
      manager.getStatementVariables(),
      manager.getStatementsTable().filterRight(EntityId::Print),
      manager.getNameTypesTable().filterRight(EntityId::Variable)
    ).select();
  attrRefTables[{EntityId::Call, AttrNameId::ProcName}] =
    filterPKBLeftRight(
      manager.getStatementVariables(),
      manager.getStatementsTable().filterRight(EntityId::Call),
      manager.getNameTypesTable().filterRight(EntityId::Procedure)
    ).select();

  // with const = const
  SECTION("with const = const, BOOLEAN") {

    std::vector<With> pass = {
      With{std::string("string"), std::string("string")},
      With{Int{"3"}, Int{"3"}},
    };
    std::vector<With> fail = {
      With{std::string("string"), std::string("diffString")},
      With{std::string("string"), std::string("STRING")},
      With{Int{"3"}, Int{"4"}},
    };
    std::vector<With> error = {
      With{std::string("string"), Int{"3"}},
      With{Int{"4"}, std::string("hellokitty")},
      With{"3", Int{"3"}},
      With{Int{"0"}, "0"},
    };

    for (const auto &with : pass) {
      DYNAMIC_SECTION("" << toString(with.first) << " = " << toString(with.second)){
        Query query;
        query.withs.push_back(with);
        query.isBooleanResult = true;

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        std::list<std::string> expected = {"true"};

        REQUIRE(evaluator.hasSemanticError(query) == false);
        REQUIRE(actual == expected);
      }
    }

    for (const auto &with : fail) {
      DYNAMIC_SECTION("" << toString(with.first) << " = " << toString(with.second)){
        Query query;
        query.withs.push_back(with);
        query.isBooleanResult = true;

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        std::list<std::string> expected = {"false"};

        REQUIRE(evaluator.hasSemanticError(query) == false);
        REQUIRE(actual == expected);
      }
    }
    for (const auto &with : error) {
      DYNAMIC_SECTION("" << toString(with.first) << " = " << toString(with.second)){
        Query query;
        query.withs.push_back(with);
        query.isBooleanResult = true;

        Evaluator evaluator(cache);
        auto actual = evaluator.evaluate(query);
        std::list<std::string> expected = {"false"};

        REQUIRE(evaluator.hasSemanticError(query) == true);
        REQUIRE(actual == expected);
      }
    }
  }

  SECTION ("synonym = synonym") {
    for (const auto &[lhsRef, table1]: attrRefTables) {
      std::string lhsName = TYPE_ID_STRING.at(lhsRef.first);
      if (lhsRef.second.has_value()) lhsName += "." + ATTRIBUTE_ID_STRING.at(lhsRef.second.value());
      for (const auto &[rhsRef, table2]: attrRefTables) {
        std::string rhsName = TYPE_ID_STRING.at(rhsRef.first);
        if (rhsRef.second.has_value()) rhsName += "." + ATTRIBUTE_ID_STRING.at(rhsRef.second.value());
        DYNAMIC_SECTION(lhsName << " = " << rhsName) {
          Query query;
          Synonym x = "x", y = "y";
          query.declarations.push_back({lhsRef.first, x});
          query.declarations.push_back({rhsRef.first, y});
          query.withs.emplace_back(AttrRef{x, lhsRef.second}, AttrRef{y, rhsRef.second});
          std::vector<AttrRef> results = {(AttrRef) x, (AttrRef) y, {x, lhsRef.second}, {y, rhsRef.second}};
          // std::vector<std::pair<EntityId, std::optional<AttrNameId>>> attrRefType
          //   = { {lhsRef.first, {}}, {rhsRef.first, {}}, lhsRef, rhsRef };
          size_t leftType = 1, rightType = 1;
          if (lhsRef.second.has_value()) leftType = ATTRIBUTE_ID_INDEX.at(lhsRef.second.value());
          if (rhsRef.second.has_value()) rightType = ATTRIBUTE_ID_INDEX.at(rhsRef.second.value());

          size_t left_col = table1.schema().size();
          ResultTable leftTable = table1, rightTable = table2;
          size_t right_col = table2.schema().size();
          if (right_col == 1) rightTable = rightTable.reschema({left_col-1});
          else if (right_col == 2) rightTable = rightTable.reschema({left_col, left_col-1});
          ResultTable resultTable = leftTable * rightTable;
          std::vector<size_t> resultCol = {leftTable.schema()[0], rightTable.schema()[0], leftTable.schema().back(), rightTable.schema().back()};

          for (size_t i = 0; i < results.size(); ++i) {
            DYNAMIC_SECTION("" << i) {
              query.result = { results[i] };
              Evaluator evaluator(cache);
              auto actual = evaluator.evaluate(query);
              if (leftType != rightType) {
                std::list<std::string> expected = {};
                REQUIRE( evaluator.hasSemanticError(query) == true );
                compareResult(actual, expected);
              } else {
                std::list<std::string> expected
                  = getNames(cache, resultTable.project({resultCol[i]}).reschema({0}));
                REQUIRE( evaluator.hasSemanticError(query) == false );
                compareResult(actual, expected);
              }
            }
          }
        }


      }
    }

  }

}
