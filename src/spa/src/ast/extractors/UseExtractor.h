#pragma once
#include <AST.h>
#include <PKBTableManager.h>
#include <UseProcessor.h>
#include <CallGraph.h>
namespace ast {
  void extractUse(const common::Program&, 
                  pkb::UseProcessor&,
                  pkb::PKBTableManager&); 
}