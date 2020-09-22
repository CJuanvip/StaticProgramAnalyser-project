#include <catch.hpp>

#include <ConstructAST.h>
#include <PKBTableManager.h>
#include <DesignExtractor.h>
#include <vector>

namespace {
void verify(const std::vector<unsigned>& expected, unsigned nameId, const pkb::PKBTable<unsigned, unsigned>& table) {
  const auto& actual = table.filterRight(nameId);
  for (auto& ans: expected) {
    REQUIRE(actual.find(ans) != actual.end());
  }
}
}
TEST_CASE("Modify TC1") {
  using namespace ast;
  using namespace common;  
  auto program = buildTestCase();
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();

  auto& modifyEntityTable = mgr.getModifyEntity();
  auto& namesTable = mgr.getNamesTable();
  std::vector<unsigned> answerX{4, 13, 18, 14, 2};
  std::vector<unsigned> answerY{5, 13, 18, 14, 2};
  std::vector<unsigned> answerFlag{1, 20, 19, 2};
  std::vector<unsigned> answerCenX{11, 16, 14, 2, 21, 19};
  std::vector<unsigned> answerCenY{12, 17, 14, 2, 22, 19};
  std::vector<unsigned> answerCount{10, 15, 14, 2};
  std::vector<unsigned> answerNormSq{23, 2};

  auto xId = *namesTable.filterRight("x").begin();
  auto yId = *namesTable.filterRight("y").begin();
  auto flagId = *namesTable.filterRight("flag").begin();
  auto cenXId = *namesTable.filterRight("cenX").begin();
  auto cenYId = *namesTable.filterRight("cenY").begin();
  auto countId = *namesTable.filterRight("count").begin();
  auto normSqId = *namesTable.filterRight("normSq").begin();

  verify(answerX, xId, modifyEntityTable);
  verify(answerY, yId, modifyEntityTable);
  verify(answerFlag, flagId, modifyEntityTable);
  verify(answerCenX, cenXId, modifyEntityTable);
  verify(answerCenY, cenYId, modifyEntityTable);
  verify(answerCount, countId, modifyEntityTable);
  verify(answerNormSq, normSqId, modifyEntityTable);

  ////////////////////////////////////
  auto procMainId = *namesTable.filterRight("main").begin();
  auto procReadPointId = *namesTable.filterRight("readPoint").begin();
  auto procComputeCentroidId = *namesTable.filterRight("computeCentroid").begin();
  std::vector<unsigned> progFlag{procMainId, procComputeCentroidId};
  std::vector<unsigned> progX{procReadPointId, procComputeCentroidId, procMainId};
  std::vector<unsigned> progY{procReadPointId, procComputeCentroidId, procMainId};
  std::vector<unsigned> progCenX{procComputeCentroidId, procMainId};
  std::vector<unsigned> progCenY{procComputeCentroidId, procMainId};
  std::vector<unsigned> progCount{procComputeCentroidId, procMainId};
  std::vector<unsigned> progNormSq{procComputeCentroidId, procMainId};
  
  verify(progX, xId, modifyEntityTable);
  verify(progY, yId, modifyEntityTable);
  verify(progFlag, flagId, modifyEntityTable);
  verify(progCenX, cenXId, modifyEntityTable);
  verify(progCenY, cenYId, modifyEntityTable);
  verify(progCount, countId, modifyEntityTable);
  verify(progNormSq, normSqId, modifyEntityTable);
}

TEST_CASE("Modify TC2") {
  using namespace ast;
  using namespace common;  
  auto program = buildTestCase2();
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();
  auto& namesTable = mgr.getNamesTable();
  auto& modifyEntityTable = mgr.getModifyEntity();
  auto idA = *namesTable.filterRight("a").begin();
  auto idB = *namesTable.filterRight("b").begin();
  auto idC = *namesTable.filterRight("c").begin();

  std::vector<unsigned> ansA{1,2,3,6,8,9,10};
  std::vector<unsigned> ansB{3, 6, 7};
  std::vector<unsigned> ansC{3, 5, 6, 9, 11, 12, 13};

  verify(ansA, idA, modifyEntityTable);
  verify(ansB, idB, modifyEntityTable);
  verify(ansC, idC, modifyEntityTable);
}

TEST_CASE("Modify TC3") {
  using namespace ast;
  using namespace common;  
  auto program = buildTestCase3();
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();
  REQUIRE(mgr.getModifyEntity().select().size() == 6 + 3); // All procedures
}

TEST_CASE("Modify TC4") {
  using namespace ast;
  using namespace common;  
  auto program = buildTestCase4();
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(program);
  designExtractor.extract();
  auto& modifyEntityTable = mgr.getModifyEntity();
  REQUIRE(mgr.getModifyEntity().selectRight().size() == 3); // only x, z, t modified, y was not
  std::vector<unsigned> ansX{11, 8, 7, 5, 4, 3, 2, 1};
  std::vector<unsigned> ansZ{10, 9, 8, 7, 5, 4, 3, 2, 1};
  std::vector<unsigned> ansT{12, 6, 4, 3, 2, 1}; 
  auto& namesTable = mgr.getNamesTable();
  auto idX = *namesTable.filterRight("x").begin();
  auto idZ = *namesTable.filterRight("z").begin();
  auto idT = *namesTable.filterRight("t").begin();
  const auto& actualX = modifyEntityTable.filterRight(idX);
  REQUIRE(actualX.size() == ansX.size() + 4); // 4 additional procedures
  for (auto ans: ansX) {
    REQUIRE(actualX.find(ans) != actualX.end());
  }
  const auto& actualZ = modifyEntityTable.filterRight(idZ);
  REQUIRE(actualZ.size() == ansZ.size() + 4); // 4 additional procedures
  for (auto ans: ansZ) {
    REQUIRE(actualZ.find(ans) != actualZ.end());
  }
  const auto& actualT = modifyEntityTable.filterRight(idT);
  REQUIRE(actualT.size() == ansT.size() + 4); // 4 additional procedures
  for (auto ans: ansT) {
    REQUIRE(actualT.find(ans) != actualT.end());
  }
}