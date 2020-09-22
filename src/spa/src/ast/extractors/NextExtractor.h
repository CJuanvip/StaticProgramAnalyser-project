#pragma once
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>

namespace ast {
  void extractNext(const pkb::ControlFlowGraph&, pkb::PKBTableManager&);
}