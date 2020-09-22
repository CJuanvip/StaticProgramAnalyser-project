#pragma once
#include <AST.h>
#include <PKBTableManager.h>
namespace ast {
  void extractParent(const common::ProcDef&, pkb::PKBTableManager&);
}