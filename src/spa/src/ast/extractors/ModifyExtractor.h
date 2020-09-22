#pragma once
#include <AST.h>
#include <ModifyProcessor.h>
namespace ast {
  void extractModify(const common::Program&, pkb::ModifyProcessor&, pkb::CallGraph&, pkb::PKBTableManager&);
}