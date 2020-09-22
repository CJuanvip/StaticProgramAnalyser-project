#pragma once
#include <AST.h>
#include <vector>
#include <iostream>
#include <memory>

namespace pkb {
  class PKBTableManager;
  class ControlFlowGraph;
  namespace cfg {
    struct WhileTask {
      unsigned whileId;
      std::shared_ptr<ControlFlowGraph> graph;
    };
    struct IfTask {
      unsigned ifId;
      std::shared_ptr<ControlFlowGraph> ifThen;
      std::shared_ptr<ControlFlowGraph> ifElse;
    };
    struct CallTask {
      unsigned callId;
      std::string calleeProcName;
    };

    using Task = std::variant<WhileTask, IfTask>;
  }
  class ControlFlowGraph {
  public:
    void initialize(unsigned numStatements);
    void process(const common::StmtLst&);
    void resolveCallTasks(PKBTableManager&);
    const std::vector<std::vector<unsigned>>& getAdjList() const;
    const std::vector<unsigned>& getBranchLinks() const;
    const std::vector<std::unordered_map<unsigned, unsigned>>& getBackwardLinks() const;
    unsigned getNumStatements() const;
    void dump(std::ostream& out = std::cout) const;
  private:
    unsigned getFirstStatementOfProcedure(const std::string& procName, PKBTableManager&);
    std::vector<unsigned> getExitStatementsOfProcedure(const std::string& procName, PKBTableManager&);
    void joinWhile(unsigned whileId, const ControlFlowGraph&);
    void joinIf(unsigned ifId, const ControlFlowGraph&, const ControlFlowGraph&);

    unsigned firstStatementId_;
    std::vector<std::vector<unsigned>> adjList_;
    std::vector<cfg::Task> tasks_;
    std::vector<cfg::CallTask> callTasks_;
    std::vector<unsigned> branchLinks_;
    std::vector<std::unordered_map<unsigned, unsigned>> backwardLinks_; 
    // end node has id = 0
  };
}