#pragma once
#include <CFGExtractor.h>
#include <ControlFlowGraph.h>
#include <AST.h>
#include <PKBTableManager.h>
namespace ast {
  void extractCFG(pkb::ControlFlowGraph&, const common::Program&, pkb::PKBTableManager&);
}