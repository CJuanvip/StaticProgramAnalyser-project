#include <ControlFlowGraph.h>
#include <SPAAssert.h>
#include <variant>
#include <Visitor.h>
#include <algorithm>
#include <PKBTableManager.h>
namespace pkb {
  using namespace common;

  void ControlFlowGraph::initialize(unsigned numStatements) {
    adjList_.resize(numStatements + 1);
    branchLinks_.resize(numStatements + 1);
    backwardLinks_.resize(numStatements + 1);
  }
  unsigned ControlFlowGraph::getNumStatements() const {
    SPA_ASSERT(adjList_.size() != 0);
    return adjList_.size() - 1;
  }
  void ControlFlowGraph::process(const StmtLst& statements) {

    auto getStatementId = [](const auto& statement) {
      return std::visit([](auto const& s) { return s.statementId;}, statement);
    };

    unsigned prevStatementId = 0;


    for(const auto& statement: statements) {
      auto currStatementId = getStatementId(statement);
      if (prevStatementId != 0) {
        adjList_[prevStatementId].push_back(currStatementId);
        prevStatementId = currStatementId;
      } else {
        prevStatementId = currStatementId;
        firstStatementId_ = currStatementId;
      }
      if (std::holds_alternative<Call>(statement)) {
        callTasks_.push_back({
          .callId = getStatementId(statement),
          .calleeProcName = std::get<Call>(statement).procedure.name
        });
      }
      if (std::holds_alternative<If>(statement)) {
        If ifs = std::get<If>(statement);
        ControlFlowGraph ifThen;
        ifThen.initialize(adjList_.size());
        ifThen.process(ifs.thenStatements);
        ControlFlowGraph ifElse;
        ifElse.initialize(adjList_.size());
        ifElse.process(ifs.elseStatements);
        cfg::IfTask ifTask;
        ifTask.ifId = ifs.statementId;
        ifTask.ifThen = std::shared_ptr<ControlFlowGraph>(new ControlFlowGraph());
        *ifTask.ifThen = ifThen;
        ifTask.ifElse = std::shared_ptr<ControlFlowGraph>(new ControlFlowGraph());
        *ifTask.ifElse = ifElse;
        cfg::Task task = ifTask;
        tasks_.push_back(task);
      }
      if (std::holds_alternative<While>(statement)) {
        While w = std::get<While>(statement);
        ControlFlowGraph subgraph;
        subgraph.initialize(adjList_.size());
        subgraph.process(w.statements);
        cfg::WhileTask whileTask;
        whileTask.whileId = w.statementId;
        whileTask.graph = std::shared_ptr<ControlFlowGraph>(new ControlFlowGraph());
        *whileTask.graph = subgraph;
        cfg::Task task = whileTask;
        tasks_.push_back(task);
      }
    }
    SPA_ASSERT(prevStatementId != 0);
    adjList_[prevStatementId].push_back(0);
    for (const auto& task: tasks_) {
      std::visit(visitor{
        [&](const cfg::WhileTask& t) { joinWhile(t.whileId, *t.graph); },
        [&](const cfg::IfTask& t) { joinIf(t.ifId, *t.ifThen, *t.ifElse); }
      }, task);
    }
    tasks_.clear();
  }

  void ControlFlowGraph::resolveCallTasks(PKBTableManager& mgr) {
    for (const auto& callTask: callTasks_) {
      auto stmtId = getFirstStatementOfProcedure(callTask.calleeProcName, mgr);
      branchLinks_[callTask.callId] = stmtId;
      auto exitIds = getExitStatementsOfProcedure(callTask.calleeProcName, mgr);
      const auto& dest = adjList_[callTask.callId];
      SPA_ASSERT(dest.size() == 1);
      for (const auto& exitId: exitIds) {
        backwardLinks_[exitId].insert({callTask.callId, dest[0]});
      }
    }
    callTasks_.clear();
  }

  const std::vector<std::vector<unsigned>>& ControlFlowGraph::getAdjList() const {
    return adjList_;
  }

  const std::vector<unsigned>& ControlFlowGraph::getBranchLinks() const {
    return branchLinks_;
  }

  const std::vector<std::unordered_map<unsigned, unsigned>>& ControlFlowGraph::getBackwardLinks() const {
    return backwardLinks_;
  }

  unsigned ControlFlowGraph::getFirstStatementOfProcedure(const std::string& procName, PKBTableManager& mgr) {
    const auto& namesTable = mgr.getNamesTable();
    const auto& filtered = namesTable.filterRight(procName);
    SPA_ASSERT(filtered.size() == 1);
    auto procId = *filtered.begin();
    const auto& procedureStatementsTable = mgr.getProcedureStatements();
    const auto& statements = procedureStatementsTable.filterLeft(procId);
    SPA_ASSERT(statements.size() > 0);
    auto it = std::min_element(statements.begin(), statements.end());
    return *it;
  }
  std::vector<unsigned> ControlFlowGraph::getExitStatementsOfProcedure(const std::string& procName, PKBTableManager& mgr) {
    std::vector<unsigned> ret;
    const auto& namesTable = mgr.getNamesTable();
    const auto& filtered = namesTable.filterRight(procName);
    SPA_ASSERT(filtered.size() == 1);
    auto procId = *filtered.begin();
    const auto& procedureStatementsTable = mgr.getProcedureStatements();
    const auto& statements = procedureStatementsTable.filterLeft(procId);
    SPA_ASSERT(statements.size() > 0);
    for (const auto& stmtId: statements) {
      const auto& dest = adjList_[stmtId];
      if (std::find(dest.begin(), dest.end(), 0) != dest.end()) {
        ret.push_back(stmtId);
      }
    }
    return ret;
  }

  void ControlFlowGraph::joinWhile(unsigned whileId, const ControlFlowGraph& subgraph) {
    const auto& subGraphAdjList = subgraph.adjList_;
    for (size_t src = 0; src < subGraphAdjList.size(); ++src) {
      for (const auto dest : subGraphAdjList[src]) {
        if (dest == 0) { //aritificial end node
          adjList_[src].push_back(whileId);
        } else {
          adjList_[src].push_back(dest);
        }
      }
    }
    adjList_[whileId].push_back(subgraph.firstStatementId_);
    callTasks_.insert(callTasks_.end(), subgraph.callTasks_.begin(), subgraph.callTasks_.end());
  }

  void ControlFlowGraph::joinIf(unsigned ifId, const ControlFlowGraph& ifThen, const ControlFlowGraph& ifElse) {
    auto& afterIfs = adjList_[ifId];
    SPA_ASSERT(afterIfs.size() == 1);
    auto afterIf = afterIfs.front();
    afterIfs.clear();
    const auto& ifThenAdjList = ifThen.adjList_;
    for (size_t src = 0; src < ifThenAdjList.size(); ++src) {
      for (const auto dest : ifThenAdjList[src]) {
        if (dest == 0) { //aritificial end node
          adjList_[src].push_back(afterIf);
        } else {
          adjList_[src].push_back(dest);
        }
      }
    }
    adjList_[ifId].push_back(ifThen.firstStatementId_);
    const auto& ifElseAdjList = ifElse.adjList_;
    for (size_t src = 0; src < ifElseAdjList.size(); ++src) {
      for (const auto dest : ifElseAdjList[src]) {
        if (dest == 0) { //aritificial end node
          adjList_[src].push_back(afterIf);
        } else {
          adjList_[src].push_back(dest);
        }
      }
    }
    adjList_[ifId].push_back(ifElse.firstStatementId_);
    callTasks_.insert(callTasks_.end(), ifThen.callTasks_.begin(), ifThen.callTasks_.end());
    callTasks_.insert(callTasks_.end(), ifElse.callTasks_.begin(), ifElse.callTasks_.end());
  }

  void ControlFlowGraph::dump(std::ostream& out) const {
    for (size_t src = 0; src < adjList_.size(); ++src) {
      out << src << ": ";
      for (const auto dest : adjList_[src]) {
        out << dest << ", ";
      }
      out << std::endl;
    }
  }
}
