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

pkb::PKBTable<unsigned, unsigned> materialize(pkb::RealtimePKBTable<unsigned, unsigned> *base) {
  pkb::PKBTable<unsigned, unsigned> result;
  for (const auto&row: base->select()) {
    result.insert(row.getLeft(), row.getRight());
  }
  return result;
}


TEST_CASE("PKB Test ClosurePairs") {
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
  };

  std::vector<std::pair<RelationId, RelationId>> closurePairs = {
    { RelationId::Follows, RelationId::FollowsT },
    { RelationId::Parent, RelationId::ParentT },
    { RelationId::Next, RelationId::NextT },
    { RelationId::Affects, RelationId::AffectsT },
    { RelationId::NextBip, RelationId::NextBipT },
  };
  int N = manager.getStatementsTable().count();
  bool baseAdjMat[N+1][N+1];
  for (const auto&[base, closure]: closurePairs) {
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(base) << "-" << RELATION_ID_STRING.at(closure)) {
      const auto &baseTable = relationTables[base];
      const auto &closureTable = relationTables[closure];

      REQUIRE(baseTable.count() <= closureTable.count());
      memset(baseAdjMat, 0, sizeof(baseAdjMat));
      for (const auto &pkbResult : baseTable.select()) {
        REQUIRE(closureTable.contains(pkbResult.getLeft(), pkbResult.getRight()));
        baseAdjMat[pkbResult.getLeft()][pkbResult.getRight()] = 1;
      }

      for (int k = 1; k <= N; ++k) {
        for (int i = 1; i <= N; ++i) {
          for (int j = 1; j <= N; ++j) {
            baseAdjMat[i][j] |= baseAdjMat[i][k] & baseAdjMat[k][j];
          }
        }
      }
      if (base == RelationId::NextBip) continue;
      for (int i = 1; i <= N; ++i) {
        for (int j = 1; j <= N; ++j) {
          if (baseAdjMat[i][j] != closureTable.contains(i, j)) {
            std::cout << toString(prog) << std::endl;
            baseTable.dump(std::cout);
            printf("i = %d, j = %d\n", i, j);
          }
          REQUIRE(baseAdjMat[i][j] == closureTable.contains(i, j));
        }
      }
    }
  }
}


TEST_CASE("PKB Test Bip") {
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
  std::unordered_map<RelationId, pkb::PKBTable<unsigned, unsigned>> relationTables = {
    { RelationId::Next, manager.getNext() },
    { RelationId::NextT, materialize(&manager.getNextClosure()) },
    { RelationId::Affects, materialize(&manager.getAffect()) },
    { RelationId::AffectsT, materialize(&manager.getAffectClosure()) },
    { RelationId::NextBip, manager.getInterproceduralNext() },
    { RelationId::NextBipT, materialize(&manager.getInterproceduralNextClosure()) },
  };

  std::vector<std::pair<RelationId, RelationId>> bipPairs = {
    { RelationId::NextT, RelationId::NextBipT }
  };

  for (const auto&[base, bip]: bipPairs) {
    DYNAMIC_SECTION("" << RELATION_ID_STRING.at(base) << "-" << RELATION_ID_STRING.at(bip)) {
      const auto &baseTable = relationTables[base];
      const auto &bipTable = relationTables[bip];

      REQUIRE(baseTable.count() <= bipTable.count());
      for (const auto &pkbResult : baseTable.select()) {
        REQUIRE(bipTable.contains(pkbResult.getLeft(), pkbResult.getRight()));
      }

    }
  }
}
