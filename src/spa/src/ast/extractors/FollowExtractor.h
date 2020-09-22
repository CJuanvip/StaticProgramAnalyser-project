#pragma once
#include <PKBTableManager.h>
#include <AST.h>
namespace ast {
  void extractFollow(const common::ProcDef&, pkb::PKBTableManager&);
}