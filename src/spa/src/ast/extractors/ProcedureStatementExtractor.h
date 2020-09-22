#pragma once
#include <AST.h>
#include <PKBTableManager.h>
/// Populate ProcedureId to StatementId mapping in said table
namespace ast {
  void populateProcedureStatementTable(const common::ProcDef&,
                                       pkb::PKBTableManager&);
}