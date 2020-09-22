#include <InterproceduralAffectClosureEvaluator.h>
#include <PKBTableManager.h>
#include <CfgVisitor.h>
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
#include <SPAAssert.h>
namespace pkb {
  namespace {
  struct TraversalInfo {
    unsigned src;
    unsigned curr;
    std::vector<unsigned> callStack;
  };
  class InterproceduralAffectIndividualExtractor: public ast::CfgVisitor {
  public:
    InterproceduralAffectIndividualExtractor() = delete;
    InterproceduralAffectIndividualExtractor(const ControlFlowGraph& cfg, pkb::PKBTableManager& mgr, CallGraph& callGraph)
    : ast::CfgVisitor(cfg), mgr_(mgr), callGraph_(callGraph) {
      // do something here, probably initialize DS
      auto size = cfg.getNumStatements() + 1; // Here, we add 1 so that index equals statement id
      affect_.resize(size);
      visited_.resize(size);
      for(auto& v: affect_) {
        v.resize(size);
      }
      const auto& statementTable = mgr.getStatementsTable();
      const auto& assignIds = statementTable.filterRight(common::EntityId::Assign);
      auto& statementVariableTable = mgr.getStatementVariables();
      auto& nameTypesTable = mgr.getNameTypesTable();
      for(const auto assignId : assignIds) {
        const auto& variableAndConstIds = statementVariableTable.filterLeft(assignId);
        for(const auto id: variableAndConstIds) {
          if (nameTypesTable.contains(id, common::EntityId::Variable)) {
            registerVariable(id);
          }
        }
      }
      numVariables_ = indexToVariableIdMap_.size();
      statementIdToModifiedIndexes_.resize(size);
      for (size_t i = 1; i < statementIdToModifiedIndexes_.size(); ++i) {
        auto& v = statementIdToModifiedIndexes_[i];
        const auto& filtered = statementTable.filterLeft(i);
        SPA_ASSERT(filtered.size() == 1);
        const auto& statementType = *filtered.begin();
        if (statementType == common::EntityId::Assign
         || statementType == common::EntityId::Read) {
          setModifiedIndexes(i, v);
        }
      }
      computeModifiesForCall(numVariables_);

      statementIdToUsedIndexes_.resize(size);
      for (size_t i = 1; i < statementIdToUsedIndexes_.size(); ++i) {
        auto& v = statementIdToUsedIndexes_[i];
        if (assignIds.find(i) != assignIds.end()) {
          setUsedIndexes(i, v);
        }
      }
    }
    void visit() override {
      while(!pendingTraversals_.empty()) {
        auto pendingTraversal = pendingTraversals_.back();
        pendingTraversals_.pop_back();
        computeAffect(pendingTraversal.src, pendingTraversal.curr, pendingTraversal.callStack);
        std::fill(visited_.begin(), visited_.end(), 0);
      }
    }
    void addPendingTraversal(const TraversalInfo& info) {
      pendingTraversals_.push_back(info);
    }
    const std::vector<std::vector<bool>>& getAffect() {
      return affect_;
    }
  private:
    void computeModifiesForCall(unsigned size) {
      const auto& callTopoOrder = callGraph_.getTopologicalOrder(); // procIds, topoOrder
      for (auto it = callTopoOrder.rbegin(); it != callTopoOrder.rend(); ++it) {
        const auto procId = *it;
        if (procId == 0) continue; // nobody calls this function
        auto firstStmtId = getFirstStatementOfProcedure(procId);
        std::fill(visited_.begin(), visited_.end(), 0);
        std::vector<bool> modifyVector(size ,0);
        computeModifiesForCall(procId, firstStmtId, modifyVector);
      }
      // transfer procIdToModifiesMap_ into statementIdToModifiedIndexes_
      for (const auto& [procId, modifiesVector]: procIdToModifiesMap_) {
        auto callStmtIds = getCallStmtIds(procId);
        for (const auto& callStmtId: callStmtIds) {
          statementIdToModifiedIndexes_[callStmtId] = modifiesVector;
        }
      }
    }

    std::vector<unsigned> getCallStmtIds(unsigned procId) {
      const auto& statementsTable = mgr_.getStatementsTable();
      const auto& statementVariablesTable = mgr_.getStatementVariables();
      const auto& calls = statementsTable.filterRight(common::EntityId::Call);
      const auto& filtered = statementVariablesTable.filterRight(procId);
      std::vector<unsigned> ret;
      for (const auto& stmtId: filtered) {
        if (calls.find(stmtId) != calls.end()) {
          ret.push_back(stmtId);
        }
      }
      return ret;
    }
    void computeModifiesForCall(unsigned procId, unsigned curr, std::vector<bool> modifyVector) {
      if (visited_[curr] || curr == 0) {
        // end of traversal path, register what we got
        if (procIdToNumRegistered_[procId] == 0) {
          for(size_t i = 0; i < modifyVector.size(); ++i) {
            auto index = modifyVector[i];
            if (index) {
              procIdToModifiesMap_[procId].insert(i);
            }
          }
          ++procIdToNumRegistered_[procId];
          return;
        }
        // if we have registered some path before, we need to and them
        for(size_t i = 0; i < modifyVector.size(); ++i) {
          auto index = modifyVector[i];
          if (!index) {
            procIdToModifiesMap_[procId].erase(i);
          }
        }
      } else {
        for (const auto& index: statementIdToModifiedIndexes_[curr]) {
          modifyVector[index] = 1;
        }
        if (isCallStatement(curr)) {
          for (const auto& index: procIdToModifiesMap_[getProcId(curr)]) {
            modifyVector[index] = 1;
          }
        }
        for (const auto& next: adjList_[curr]) {
          visited_[curr] = true;
          computeModifiesForCall(procId, next, modifyVector);
          visited_[curr] = false;
        }
      }
    }
    bool isCallStatement(const unsigned stmtId) {
      const auto& statementsTable = mgr_.getStatementsTable();
      const auto& filtered = statementsTable.filterLeft(stmtId);
      SPA_ASSERT(filtered.size() == 1);
      return *filtered.begin() == common::EntityId::Call;
    }
    unsigned getProcId(const unsigned callStmtId) {
      const auto& statementVariablesTable = mgr_.getStatementVariables();
      const auto& filtered = statementVariablesTable.filterLeft(callStmtId);
      SPA_ASSERT(filtered.size() == 1);
      return *filtered.begin();
    }
    unsigned getFirstStatementOfProcedure(const unsigned procId) {
      const auto& procedureStatementsTable = mgr_.getProcedureStatements();
      const auto& statements = procedureStatementsTable.filterLeft(procId);
      SPA_ASSERT(statements.size() > 0);
      auto it = std::min_element(statements.begin(), statements.end());
      return *it;
    }
    // Precondition: src is an assign statement's id
    void computeAffect(unsigned src, unsigned curr, std::vector<unsigned>& callStack) {
      unsigned modified;
      initializeStatusWithModify(curr, modified);
      for (const auto dest: adjList_[curr]) {
        if (dest != 0) { 
          computeAffect(src, dest, modified, callStack);
          std::fill(visited_.begin(), visited_.end(), 0);
        } else {
          if (callStack.empty()) {
            auto dests = getDests(curr);
            for (const auto& dest: dests) {
              computeAffect(src, dest, modified, callStack);
              std::fill(visited_.begin(), visited_.end(), 0);
            }
          } else {
            auto callStackCopy = callStack;
            auto callerId = callStackCopy.back();
            auto currCopy = curr;
            unsigned res = 0;
            while(!callStackCopy.empty() && (res = backwardLinks_[currCopy].at(callerId)) == 0) {
              callStackCopy.pop_back();
              currCopy = callerId;
              callerId = callStackCopy.back();
            }
            if (!callStackCopy.empty()) callStackCopy.pop_back();
            if (callStackCopy.empty() && res == 0) {
              auto dests = getDests(currCopy);
              for (const auto& dest: dests) {
                computeAffect(src, dest, modified, callStackCopy);
                std::fill(visited_.begin(), visited_.end(), 0);
              }
            } else {
              computeAffect(src, res, modified, callStackCopy);
              std::fill(visited_.begin(), visited_.end(), 0);
            }
            
          }
        }
      }
    }
    void computeAffect(unsigned src, unsigned dest,
                       unsigned modified,
                       std::vector<unsigned>& callStack) {
      if (visited_[dest]) {
        return;
      }
      // this is a call statement, we can recurse down,
      // by ignoring the precomputed modifies vector for the callee function as a whole
      // and let traversal does its job
      if (branchLinks_[dest] != 0) {
        callStack.push_back(dest);
        visited_[dest] = true;
        computeAffect(src, branchLinks_[dest], modified, callStack);
        visited_[dest] = false;
        callStack.pop_back();
      }
      bool isAffect = processStatusWithUses(dest, modified);
      if (isAffect) {
        if (!hasSeen(src, dest, callStack)) {
          pendingTraversals_.push_back({src, dest, callStack});
        }
        affect_[src][dest] = true;
      }

      bool shouldContinue = processStatusWithModifies(dest, modified);
      if (!shouldContinue) {
        return;
      }
      // we can also mimic the call statement via a specially crafted modifies vector
      // so it is still safe to traverse all adjList edges
      for (const auto next: adjList_[dest]) {
        if (next == 0) {
          if (callStack.empty()) {
                // we only backtrack if callStack is empty
            const auto dests = getDests(dest);
            for (const auto& d: dests) {
              visited_[dest] = true;
              computeAffect(src, d, modified, callStack);
              visited_[dest] = false;
            }
          } else {

            // callStack is not empty, so we need to unwind it until we see a valid nextStatement
            auto callStackCopy = callStack;
            auto callerId = callStackCopy.back();
            auto currCopy = dest;
            unsigned result = 0;
            while(!callStackCopy.empty() && ((result = backwardLinks_[currCopy].at(callerId)) == 0)) {
              callStackCopy.pop_back();
              currCopy = callerId;
              callerId = callStackCopy.back();
            }
            if (!callStackCopy.empty()) {
              callStackCopy.pop_back();
            }
            if (callStackCopy.empty() && result == 0) {
              auto dests = getDests(currCopy);
              for (const auto& destination: dests) {
                visited_[dest] = true;
                computeAffect(src, destination, modified, callStackCopy);
                visited_[dest] = false;
              }
            } else {
              visited_[dest] = true;
              computeAffect(src, result, modified, callStackCopy);
              visited_[dest] = false;
            }

          }
        }
        visited_[dest] = true;
        computeAffect(src, next, modified, callStack);
        visited_[dest] = false;
      }

    }

    bool processStatusWithUses(unsigned statementId, unsigned modified) {
      const auto& usesSet = statementIdToUsedIndexes_[statementId];
      for (const auto& index: usesSet) {
        if (index == modified) {
          return true;
        }
      }
      return false;
    }

    bool processStatusWithModifies(unsigned statementId, unsigned modified) {
      const auto& modifiesSet = statementIdToModifiedIndexes_[statementId];
      if (modifiesSet.find(modified) != modifiesSet.end()) {
        return false;
      } else {
        return true;
      }
    }

    void initializeStatusWithModify(unsigned statementId, unsigned& modified) {
      const auto& modifiesSet = statementIdToModifiedIndexes_[statementId];
      SPA_ASSERT(modifiesSet.size() == 1);
      modified = *modifiesSet.cbegin();
    }

    void registerVariable(unsigned variableId) {
      if (variableIdToIndexMap_.find(variableId) == variableIdToIndexMap_.end()) {
        auto index = indexToVariableIdMap_.size();
        indexToVariableIdMap_.push_back(variableId);
        variableIdToIndexMap_.insert({variableId, index});
      }
    }
    void setModifiedIndexes(unsigned statementId, std::unordered_set<unsigned>& modifiedSet) {
      const auto& modifyTable = mgr_.getModifyEntity();
      const auto& modifiedVariableIds = modifyTable.filterLeft(statementId);
      for (const auto varId : modifiedVariableIds) {
        if (variableIdToIndexMap_.find(varId) != variableIdToIndexMap_.end()) {
          modifiedSet.insert(variableIdToIndexMap_[varId]);
        }
      }
    }
    void setUsedIndexes(unsigned statementId, std::unordered_set<unsigned>& usedSet) {
      const auto& usedTable = mgr_.getUsesEntityVariableTable();
      const auto& usedVariableIds = usedTable.filterLeft(statementId);
      for (const auto varId : usedVariableIds) {
        if (variableIdToIndexMap_.find(varId) != variableIdToIndexMap_.end()) {
          usedSet.insert(variableIdToIndexMap_[varId]);
        }
      }
    }
    bool hasSeen(unsigned src, unsigned dest, const std::vector<unsigned>& callStack) {
      if (seen_.find(src) == seen_.end()) {
        seen_[src][dest].push_back(callStack);
        return false;
      }
      if (seen_[src].find(dest) == seen_[src].end()) {
        seen_[src][dest].push_back(callStack);
        return false;
      }
      for (const auto& seen: seen_[src][dest]) {
        if (seen == callStack) {
          return true;
        }
      }
      seen_[src][dest].push_back(callStack);
      return false;
    }
    std::vector<std::vector<bool>> affect_;
    // those only appears in assignment will have an index; index is 0-based

    std::vector<unsigned> indexToVariableIdMap_;
    std::unordered_map<unsigned, unsigned> variableIdToIndexMap_;

    // Only those variable registered above will be added here
    std::vector<std::unordered_set<unsigned>> statementIdToModifiedIndexes_;
    std::vector<std::unordered_set<unsigned>> statementIdToUsedIndexes_;
    unsigned numVariables_;
    std::vector<bool> visited_;
    std::unordered_map<unsigned, unsigned> procIdToNumRegistered_;
    std::unordered_map<unsigned, std::unordered_set<unsigned>> procIdToModifiesMap_;
    pkb::PKBTableManager& mgr_;
    pkb::CallGraph& callGraph_;
    std::vector<TraversalInfo> pendingTraversals_;
    std::unordered_map<unsigned, std::unordered_map<unsigned, std::vector<std::vector<unsigned>>>> seen_;
  };
  class InterproceduralAffectClosureExtractor {
  public:
    InterproceduralAffectClosureExtractor(const ControlFlowGraph& cfg, pkb::PKBTableManager& mgr, CallGraph& callGraph) 
    : mgr_(mgr)
    , affectExtractor_(cfg, mgr, callGraph) {
      const auto size = mgr_.getStatementsTable().count() + 1;
      affectsClosure_.assign(size, std::vector<bool>(size, false));
    }
    const std::vector<std::vector<bool>> getAffectsClosure() const {
      return affectsClosure_;
    }
    void visit() {
      const auto& assignIds = mgr_.getStatementsTable().filterRight(common::EntityId::Assign);
      for (const auto assignId : assignIds) {
        affectExtractor_.addPendingTraversal({assignId, assignId, {}});
      }
      affectExtractor_.visit();
      affectsClosure_ = affectExtractor_.getAffect();
    }
  private:
    
    pkb::PKBTableManager& mgr_;
    std::vector<std::vector<bool>> affectsClosure_;
    InterproceduralAffectIndividualExtractor affectExtractor_;
  };
  }// namespace anonymous
void InterproceduralAffectClosureEvaluator::initialize(PKBTableManager& mgr, ControlFlowGraph& cfg, CallGraph& callGraph) {
  numStatements_ = mgr.getStatementsTable().count();
  mgr_ = &mgr;
  cfg_ = &cfg;
  callGraph_ = &callGraph;
}
bool InterproceduralAffectClosureEvaluator::select(PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralAffectClosureEvaluator::filterLeft(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralAffectClosureEvaluator::filterRight(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralAffectClosureEvaluator::contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
void InterproceduralAffectClosureEvaluator::reset() {
}
void InterproceduralAffectClosureEvaluator::extract(PKBTable<unsigned, unsigned>& table) {
  InterproceduralAffectClosureExtractor visitor(*cfg_, *mgr_, *callGraph_);
  visitor.visit();
  const auto& affectsClosure = visitor.getAffectsClosure();
  for (size_t i = 1; i <= numStatements_; ++i) {
    for (size_t j = 1; j <= numStatements_; ++j) {
      if (affectsClosure[i][j]) {
        table.insert(i,j);
      }
    }
  }
}
}
