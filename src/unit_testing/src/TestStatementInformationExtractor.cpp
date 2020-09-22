#include <catch.hpp>
#include <StatementInformationExtractor.h>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <NameExtractor.h>
#include <PKBTableManager.h>
namespace {
void verify(const std::vector<unsigned>& answers, 
                             unsigned nameId,
                             pkb::PKBTable<unsigned, unsigned>& table) {
  const auto& actual = table.filterRight(nameId);
  REQUIRE(actual.size() == answers.size());
  for (const auto ans: answers) {
    REQUIRE(actual.find(ans) != actual.end());
  }
}
}// anonymous
TEST_CASE("Assign and StatementVariable tables are populated correctly") {
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  // Prereq: statementId, procedureName, variableName
  normalizeStatementId(program, mgr);
  for(const auto& procedure: program) {
    transferProcedureName(procedure, mgr);
  }
  NameExtractor extractor(mgr);
  StatementInformationExtractor infoExtractor(mgr);
  for (const auto& procedure: program) {
    for(const auto& statement: procedure.statements) {
      std::visit(extractor, statement);
      // one can immediately extract info as long as that procedure has name populated
      std::visit(infoExtractor, statement);
    }
  }

  std::vector<unsigned> statementWithFlag{1, 6, 20};
  std::vector<unsigned> statementWithX{4, 14, 16};
  std::vector<unsigned> statementWithY{5, 14, 17};
  std::vector<unsigned> statementWithCenX{7, 11, 16, 21, 23};
  std::vector<unsigned> statementWithCenY{8, 12, 17, 22, 23};
  std::vector<unsigned> statementWithNormSq{9, 23};
  std::vector<unsigned> statementWithCount{10, 15, 19, 21, 22};
  std::vector<unsigned> statementWith0{1, 10, 11, 12, 14, 19};
  std::vector<unsigned> statementWith1{15, 20};

  auto& namesTable = mgr.getNamesTable();
  unsigned idFlag = *namesTable.filterRight("flag").begin();
  unsigned idX = *namesTable.filterRight("x").begin();
  unsigned idY = *namesTable.filterRight("y").begin();
  unsigned idCenX = *namesTable.filterRight("cenX").begin();
  unsigned idCenY = *namesTable.filterRight("cenY").begin();
  unsigned idNormSq = *namesTable.filterRight("normSq").begin();
  unsigned idCount = *namesTable.filterRight("count").begin();
  unsigned id0 = *namesTable.filterRight("0").begin();
  unsigned id1 = *namesTable.filterRight("1").begin();

  auto& statementVariableTable = mgr.getStatementVariables();
  verify(statementWithFlag, idFlag, statementVariableTable);
  verify(statementWithX, idX, statementVariableTable);
  verify(statementWithY, idY, statementVariableTable);
  verify(statementWithCenX, idCenX, statementVariableTable);
  verify(statementWithCenY, idCenY, statementVariableTable);
  verify(statementWithNormSq, idNormSq, statementVariableTable);
  verify(statementWithCount, idCount, statementVariableTable);
  verify(statementWith0, id0, statementVariableTable);
  verify(statementWith1, id1, statementVariableTable);
  
  std::vector<unsigned> assignLhsWithFlag{1, 20};
  std::vector<unsigned> assignLhsWithX{};
  std::vector<unsigned> assignLhsWithY{};
  std::vector<unsigned> assignLhsWithCenX{11, 16, 21};
  std::vector<unsigned> assignLhsWithCenY{12, 17, 22};
  std::vector<unsigned> assignLhsWithNormSq{23};
  std::vector<unsigned> assignLhsWithCount{10, 15};
  std::vector<unsigned> assignLhsWith0{};
  std::vector<unsigned> assignLhsWith1{};

  auto& assignLhsTable =  mgr.getAssignLhs();
  verify(assignLhsWithFlag, idFlag, assignLhsTable);
  verify(assignLhsWithX, idX, assignLhsTable);
  verify(assignLhsWithY, idY, assignLhsTable);
  verify(assignLhsWithCenX, idCenX, assignLhsTable);
  verify(assignLhsWithCenY, idCenY, assignLhsTable);
  verify(assignLhsWithNormSq, idNormSq, assignLhsTable);
  verify(assignLhsWithCount, idCount, assignLhsTable);
  verify(assignLhsWith0, id0, assignLhsTable);
  verify(assignLhsWith1, id1, assignLhsTable);

  std::vector<unsigned> assignRhsWithFlag{};
  std::vector<unsigned> assignRhsWithX{16};
  std::vector<unsigned> assignRhsWithY{17};
  std::vector<unsigned> assignRhsWithCenX{16, 21, 23};
  std::vector<unsigned> assignRhsWithCenY{17, 22, 23};
  std::vector<unsigned> assignRhsWithNormSq{};
  std::vector<unsigned> assignRhsWithCount{15, 21, 22};
  std::vector<unsigned> assignRhsWith0{1, 10, 11, 12};
  std::vector<unsigned> assignRhsWith1{15, 20};

  auto& assignRhsTable =  mgr.getAssignRhs();
  verify(assignRhsWithFlag, idFlag, assignRhsTable);
  verify(assignRhsWithX, idX, assignRhsTable);
  verify(assignRhsWithY, idY, assignRhsTable);
  verify(assignRhsWithCenX, idCenX, assignRhsTable);
  verify(assignRhsWithCenY, idCenY, assignRhsTable);
  verify(assignRhsWithNormSq, idNormSq, assignRhsTable);
  verify(assignRhsWithCount, idCount, assignRhsTable);
  verify(assignRhsWith0, id0, assignRhsTable);
  verify(assignRhsWith1, id1, assignRhsTable);
  
  REQUIRE(mgr.getAssignRhsAst().select().size() == 11);
}