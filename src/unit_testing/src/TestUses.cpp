#include <catch.hpp>

#include <PKBTableManager.h>
#include <ConstructAST.h>
#include <DesignExtractor.h>

TEST_CASE("Test Uses TC1") {
  using namespace ast;
  using namespace common;
  auto program = buildTestCase();

  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();
  
  auto& namesTable = mgr.getNamesTable();

  auto xId = *namesTable.filterRight("x").begin();
  auto yId = *namesTable.filterRight("y").begin();
  auto flagId = *namesTable.filterRight("flag").begin();
  auto cenXId = *namesTable.filterRight("cenX").begin();
  auto cenYId = *namesTable.filterRight("cenY").begin();
  auto countId = *namesTable.filterRight("count").begin();
  auto normSqId = *namesTable.filterRight("normSq").begin();
  
  unsigned Stmt1 = 1;
  unsigned Stmt2 = 2;
  unsigned Stmt3 = 3;
  unsigned Stmt4 = 4;
  unsigned Stmt5 = 5;
  unsigned Stmt6 = 6;
  unsigned Stmt7 = 7;
  unsigned Stmt8 = 8;
  unsigned Stmt9 = 9;
  unsigned Stmt10 = 10;
  unsigned Stmt11 = 11;
  unsigned Stmt12 = 12;
  unsigned Stmt13 = 13;
  unsigned Stmt14 = 14;
  unsigned Stmt15 = 15;
  unsigned Stmt16 = 16;
  unsigned Stmt17 = 17;
  unsigned Stmt18 = 18;
  unsigned Stmt19 = 19;
  unsigned Stmt20 = 20;
  unsigned Stmt21 = 21;
  unsigned Stmt22 = 22;
  unsigned Stmt23 = 23;

  std::vector<unsigned> Stmt1Uses{};
  std::vector<unsigned> Stmt2Uses = {xId, yId, cenXId, cenYId, countId};
  std::vector<unsigned> Stmt3Uses = {flagId, cenXId, cenYId, normSqId};
  std::vector<unsigned> Stmt4Uses = {};
  std::vector<unsigned> Stmt5Uses = {};
  std::vector<unsigned> Stmt6Uses = {flagId};
  std::vector<unsigned> Stmt7Uses = {cenXId};
  std::vector<unsigned> Stmt8Uses = {cenYId};
  std::vector<unsigned> Stmt9Uses = {normSqId};
  std::vector<unsigned> Stmt10Uses = {};
  std::vector<unsigned> Stmt11Uses = {};
  std::vector<unsigned> Stmt12Uses = {};
  std::vector<unsigned> Stmt13Uses = {};
  std::vector<unsigned> Stmt14Uses = {xId, yId, cenXId, cenYId, countId};
  std::vector<unsigned> Stmt15Uses = {countId};
  std::vector<unsigned> Stmt16Uses = {cenXId, xId};
  std::vector<unsigned> Stmt17Uses = {cenYId, yId};
  std::vector<unsigned> Stmt18Uses = {};
  std::vector<unsigned> Stmt19Uses = {countId, cenXId, cenYId};
  std::vector<unsigned> Stmt20Uses = {};
  std::vector<unsigned> Stmt21Uses = {cenXId, countId};
  std::vector<unsigned> Stmt22Uses = {cenYId, countId};
  std::vector<unsigned> Stmt23Uses = {cenXId, cenYId};

  const auto& testStmt1Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt1);
  for (auto& ans: Stmt1Uses) {
    REQUIRE(testStmt1Uses.find(ans) != testStmt1Uses.end());
  }
  const auto& testStmt2Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt2);
  for (auto& ans: Stmt2Uses) {
    REQUIRE(testStmt2Uses.find(ans) != testStmt2Uses.end());
  }
  const auto& testStmt3Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt3);
  for (auto& ans: Stmt3Uses) {
    REQUIRE(testStmt3Uses.find(ans) != testStmt3Uses.end());
  }
  const auto& testStmt4Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt4);
  for (auto& ans: Stmt4Uses) {
    REQUIRE(testStmt4Uses.find(ans) != testStmt4Uses.end());
  }
  const auto& testStmt5Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt5);
  for (auto& ans: Stmt5Uses) {
    REQUIRE(testStmt5Uses.find(ans) != testStmt5Uses.end());
  }
  const auto& testStmt6Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt6);
  for (auto& ans: Stmt6Uses) {
    REQUIRE(testStmt6Uses.find(ans) != testStmt6Uses.end());
  }
  const auto& testStmt7Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt7);
  for (auto& ans: Stmt7Uses) {
    REQUIRE(testStmt7Uses.find(ans) != testStmt7Uses.end());
  }
  const auto& testStmt8Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt8);
  for (auto& ans: Stmt8Uses) {
    REQUIRE(testStmt8Uses.find(ans) != testStmt8Uses.end());
  }
  const auto& testStmt9Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt9);
  for (auto& ans: Stmt9Uses) {
    REQUIRE(testStmt9Uses.find(ans) != testStmt9Uses.end());
  }
  const auto& testStmt10Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt10);
  for (auto& ans: Stmt10Uses) {
    REQUIRE(testStmt10Uses.find(ans) != testStmt10Uses.end());
  }
  const auto& testStmt11Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt11);
  for (auto& ans: Stmt11Uses) {
    REQUIRE(testStmt11Uses.find(ans) != testStmt11Uses.end());
  }
  const auto& testStmt12Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt12);
  for (auto& ans: Stmt12Uses) {
    REQUIRE(testStmt12Uses.find(ans) != testStmt12Uses.end());
  }
  const auto& testStmt13Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt13);
  for (auto& ans: Stmt13Uses) {
    REQUIRE(testStmt13Uses.find(ans) != testStmt13Uses.end());
  }
  const auto& testStmt14Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt14);
  for (auto& ans: Stmt14Uses) {
    REQUIRE(testStmt14Uses.find(ans) != testStmt14Uses.end());
  }
  const auto& testStmt15Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt15);
  for (auto& ans: Stmt15Uses) {
    REQUIRE(testStmt15Uses.find(ans) != testStmt15Uses.end());
  }
  const auto& testStmt16Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt16);
  for (auto& ans: Stmt16Uses) {
    REQUIRE(testStmt16Uses.find(ans) != testStmt16Uses.end());
  }
  const auto& testStmt17Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt17);
  for (auto& ans: Stmt17Uses) {
    REQUIRE(testStmt17Uses.find(ans) != testStmt17Uses.end());
  }
  const auto& testStmt18Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt18);
  for (auto& ans: Stmt18Uses) {
    REQUIRE(testStmt18Uses.find(ans) != testStmt18Uses.end());
  }
  const auto& testStmt19Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt19);
  for (auto& ans: Stmt19Uses) {
    REQUIRE(testStmt19Uses.find(ans) != testStmt19Uses.end());
  }
  const auto& testStmt20Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt20);
  for (auto& ans: Stmt20Uses) {
    REQUIRE(testStmt20Uses.find(ans) != testStmt20Uses.end());
  }
  const auto& testStmt21Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt21);
  for (auto& ans: Stmt21Uses) {
    REQUIRE(testStmt21Uses.find(ans) != testStmt21Uses.end());
  }
  const auto& testStmt22Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt22);
  for (auto& ans: Stmt22Uses) {
    REQUIRE(testStmt22Uses.find(ans) != testStmt22Uses.end());
  }
  const auto& testStmt23Uses = mgr.getUsesEntityVariableTable().filterLeft(Stmt23);
  for (auto& ans: Stmt23Uses) {
    REQUIRE(testStmt23Uses.find(ans) != testStmt23Uses.end());
  }
}

TEST_CASE("Test Uses TC2") {
  using namespace ast;
  using namespace common;
  auto program = buildTestCase5();

  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();


  auto& namesTable = mgr.getNamesTable();
  auto varBId = *namesTable.filterRight("varB").begin();
  auto condVarBId = *namesTable.filterRight("condVarB").begin();
  auto varFId = *namesTable.filterRight("varF").begin();

  auto procAId = *namesTable.filterRight("A").begin();
  auto procBId = *namesTable.filterRight("B").begin();
  auto procCId = *namesTable.filterRight("C").begin();
  auto procDId = *namesTable.filterRight("D").begin();
  auto procEId = *namesTable.filterRight("E").begin();
  auto procFId = *namesTable.filterRight("F").begin();

  unsigned stmt1Id = 1;
  unsigned stmt2Id = 2;
  unsigned stmt3Id = 3;
  unsigned stmt4Id = 4;
  unsigned stmt5Id = 5;
  unsigned stmt6Id = 6;
  unsigned stmt7Id = 7;
  unsigned stmt8Id = 8;
  unsigned stmt9Id = 9;
  unsigned stmt10Id = 10;

  std::vector<unsigned> entitiesUsingVarB{stmt1Id, stmt3Id, procAId, procBId};
  std::vector<unsigned> entitiesUsingVarF{procAId, procBId, procCId, procDId, procEId, procFId,
                                          stmt1Id, stmt2Id, stmt4Id, stmt5Id, stmt6Id, stmt7Id, stmt8Id, stmt9Id, stmt10Id};
  std::vector<unsigned> entitiesUsingCondVarB{stmt4Id, stmt1Id, procAId, procBId};

  const auto& testEntitiesUsingVarB = mgr.getUsesEntityVariableTable().filterRight(varBId);
  for (auto& ans: entitiesUsingVarB) {
    REQUIRE(testEntitiesUsingVarB.find(ans) != testEntitiesUsingVarB.end());
  }
  REQUIRE(testEntitiesUsingVarB.size() == entitiesUsingVarB.size());

  const auto& testEntitiesUsingVarF = mgr.getUsesEntityVariableTable().filterRight(varFId);
  for (auto& ans: entitiesUsingVarF) {
    REQUIRE(testEntitiesUsingVarF.find(ans) != testEntitiesUsingVarF.end());
  }
  REQUIRE(testEntitiesUsingVarF.size() == entitiesUsingVarF.size());

  const auto& testEntitiesUsingCondVarB = mgr.getUsesEntityVariableTable().filterRight(condVarBId);
  for (auto& ans: entitiesUsingCondVarB) {
    REQUIRE(testEntitiesUsingCondVarB.find(ans) != testEntitiesUsingCondVarB.end());
  }
  REQUIRE(testEntitiesUsingCondVarB.size() == entitiesUsingCondVarB.size());
}
