#pragma once
#include <AST.h>
#include <PKBTableManager.h>
/// Extracts Procedure's ProcedureName into Name and NameType table
namespace ast {
  void transferProcedureName(const common::ProcDef&, pkb::PKBTableManager&);
}