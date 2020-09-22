#pragma once
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
namespace ast {
  void extractInterproceduralNext(const pkb::ControlFlowGraph&, pkb::PKBTableManager&);
}