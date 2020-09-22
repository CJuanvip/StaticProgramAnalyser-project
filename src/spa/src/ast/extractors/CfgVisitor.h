#pragma once
#include <ControlFlowGraph.h>
namespace ast {
  class CfgVisitor {
  public:
    virtual void visit() = 0;
  protected:
    CfgVisitor(const pkb::ControlFlowGraph& cfg) 
    : adjList_(cfg.getAdjList())
    , branchLinks_(cfg.getBranchLinks())
    , backwardLinks_(cfg.getBackwardLinks()) { }
    std::vector<unsigned> getDests(unsigned src) {
      // This is the process where it no longer calls, but when it returns, encounters caller's end
      // however, we can allow caller to return, until we see something
      std::vector<unsigned> ret;
      const auto& backwards = backwardLinks_[src];
      for (const auto& [callerStmtId, nextStmtId]: backwards) {
        if (nextStmtId != 0) {
          ret.push_back(nextStmtId);
        } else {
          // recurse
          const auto& recurse = getDests(callerStmtId);
          ret.insert(ret.end(), recurse.begin(), recurse.end());
        }
      }
      return ret;
    }
    const std::vector<std::vector<unsigned>>& adjList_;
    const std::vector<unsigned>& branchLinks_;
    const std::vector<std::unordered_map<unsigned, unsigned>> backwardLinks_;
  };
}