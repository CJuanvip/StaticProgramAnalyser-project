#pragma once
#include <AST.h>
#include <PKBTableManager.h>
/// Normalizes statement id for all statements in a Program
/// Also inserts statementId into Name and NameType table
namespace ast {
  void normalizeStatementId(common::Program& program, pkb::PKBTableManager& mgr);
}