#include <catch.hpp>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <ProcedureNameExtractor.h>
#include <ProcedureStatementExtractor.h>
#include <vector>
namespace {
void verify(unsigned procId, unsigned startingStmtId, unsigned endStmtId, const pkb::PKBTable<unsigned, unsigned>& table) {
  const auto& set = table.filterLeft(procId);
  for (unsigned i = startingStmtId; i <= endStmtId; ++i) {
    REQUIRE(set.find(i) != set.end());
  }
}
} // anonymous
TEST_CASE("Procedure Statement Table Populated Correctly") {
  using namespace ast;
  pkb::PKBTableManager mgr;
  auto program = buildTestCase();
  // prerequisite first
  normalizeStatementId(program, mgr);
  for (const auto& procedure: program) {
    // prerequisite first
    transferProcedureName(procedure, mgr);
    // actual work
    populateProcedureStatementTable(procedure, mgr);
  }
  // VERIFY
  auto& namesTable = mgr.getNamesTable();
  auto& procedureStatementTable = mgr.getProcedureStatements();
  auto procMainId = *namesTable.filterRight("main").begin();
  auto procReadPointId = *namesTable.filterRight("readPoint").begin();
  auto procPrintResultsId = *namesTable.filterRight("printResults").begin();
  auto procComputeCentroidId = *namesTable.filterRight("computeCentroid").begin();
  verify(procMainId, 1, 3, procedureStatementTable);
  verify(procReadPointId, 4, 5, procedureStatementTable);
  verify(procPrintResultsId, 6, 9, procedureStatementTable);
  verify(procComputeCentroidId, 10, 23, procedureStatementTable);
}