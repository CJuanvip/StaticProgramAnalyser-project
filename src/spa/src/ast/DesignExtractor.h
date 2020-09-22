#pragma once
#include <PKBTableManager.h>
#include <CallGraph.h>
#include <ModifyProcessor.h>
#include <UseProcessor.h>
#include <AST.h>
#include <ControlFlowGraph.h>
namespace ast {
  class DesignExtractor {
  public:
    DesignExtractor(pkb::PKBTableManager&);
    void initialize(const common::Program&);
    void extract();
    void clearCache();
  private:

    common::Program program_; // Root of AST

    pkb::PKBTableManager& mgr_;
    pkb::CallGraph callGraph_;
    pkb::ModifyProcessor modifyProcessor_;
    pkb::UseProcessor useProcessor_;

    pkb::ControlFlowGraph controlFlowGraph_;
  };
}