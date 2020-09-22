#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <optional>

namespace pkb {
  class CallGraph {
  public:
    using ProcedureId = unsigned;
    void initialize(unsigned numProcedures) {
      adjList_.resize(numProcedures);
      transposeAdjList_.resize(numProcedures);
      procedureIds_.resize(numProcedures);
      numProcedures_ = numProcedures;
    }

    void addFunctionCall(unsigned callerId, unsigned calleeId) {
      auto callerIndex = getIndex(callerId);
      auto& destVertices = adjList_[callerIndex];
      // since call to nonexistent procedure will throw beforehand, this will not out of range
      auto calleeIndex = getIndex(calleeId);
      destVertices.insert(calleeIndex);
      transposeAdjList_[calleeIndex].insert(callerIndex);
    }
    bool hasCycle() const {
      std::vector<bool> visited;
      visited.resize(numProcedures_);
      bool hasCycle = false;
      for (size_t i = 0; i < numProcedures_; ++i) {
        std::vector<int> recurStack;
        hasCycle |= checkCycle(i, visited, recurStack);
      }
      return hasCycle;
    }

    std::vector<std::pair<ProcedureId, ProcedureId>> getDirectCallerCalleePairs() const {
      std::vector<std::pair<ProcedureId, ProcedureId>> ret;
      for (int i = 0 ; i < nextIndex_; ++i) {
        const auto& dests = adjList_[i];
        for(auto dest: dests) {
          ret.push_back({getProcedureId(i), getProcedureId(dest)});
        }
      }
      return ret;
    }

    std::vector<std::pair<ProcedureId, ProcedureId>> getAllCallerCalleePairs() const {
      std::vector<std::pair<ProcedureId, ProcedureId>> results;
      for (int i = 0; i < nextIndex_; i++) {
        std::vector<bool> reachableVertices = getReachableVertices(i);
        // append all results
        // need to use size_t?
        for (size_t j = 0; j < reachableVertices.size(); j++) {
          if (reachableVertices[j]) {
            results.push_back(std::pair(getProcedureId(i), getProcedureId(j)));
          }
        }
      }
      return results;
    }


    std::vector<ProcedureId> getTopologicalOrder() const {
      auto topologicalIndexes = getTopologicalIndexes();
      std::vector<ProcedureId> ret;
      for (auto index: topologicalIndexes) {
        ret.push_back(getProcedureId(index));
      }
      return ret;
    }
    std::vector<ProcedureId> getCallerIds(ProcedureId calleeId) const {
      auto calleeIndex = getIndex(calleeId);
      // if the procedureId is not found, i.e. not registered in the callGraph,
      // then it would not have any callers.
      if (!calleeIndex) {
        return {};
      }
      auto callerIndexes = transposeAdjList_[calleeIndex.value()];
      std::vector<ProcedureId> ret;
      for (auto index: callerIndexes) {
        ret.push_back(getProcedureId(index));
      }
      return ret;
    }
    void dump(std::ostream& out = std::cout) const {
      for (size_t i = 0; i < adjList_.size(); ++i) {
        out << getProcedureId(i) << ": ";
        for (auto j: adjList_[i]) {
          out << getProcedureId(j) << ", ";
        }
        out << std::endl;
      }
    }
  private:
    std::vector<int> getTopologicalIndexes() const {
      std::vector<int> sorted;
      std::vector<bool> visited;
      visited.resize(numProcedures_);
      for (size_t i = 0; i < numProcedures_; ++i) {
        if (!visited[i]) {
          topoSort(i, visited, sorted);
        }
      }
      std::reverse(sorted.begin(), sorted.end());
      return sorted;
    }
    void topoSort(size_t index, std::vector<bool>& visited, std::vector<int>& sorted) const {
      visited[index] = true;
      for (auto dest: adjList_[index]) {
        if (!visited[dest]) {
          topoSort(dest, visited, sorted);
        }
      }
      sorted.push_back(index);
    }
    bool checkCycle(unsigned src, std::vector<bool>& visited, std::vector<int>& recurStack) const {
      visited[src] = true;
      auto& destVertices = adjList_[src];
      bool hasCycle = false;
      recurStack.push_back(src);
      for(auto& dest: destVertices) {
        if (std::find(recurStack.begin(), recurStack.end(), dest) != recurStack.end()) {
          hasCycle = true;
          break;
        }
        if (visited[dest]) {
          continue;
        }
        hasCycle |= checkCycle(dest, visited, recurStack);
      }
      recurStack.pop_back();
      return hasCycle;
    }

    // assumes that the graph is a Tree or forest
    // can be improved to prevent return by value?
    std::vector<bool> getReachableVertices(int start) const {
      // std::cout << "getReachableVertices from " << start << "\n";
      std::vector<bool> visited;
      visited.resize(numProcedures_);

      std::vector<int> stack;
      stack.push_back(start);
      visited[start] = true;

      while(!stack.empty()) {
        auto& currentVertex = stack.back();
        stack.pop_back();
        const auto& neighbors = adjList_[currentVertex];
        for(const auto& nbr: neighbors) {
          if (!visited[nbr]) {
            stack.push_back(nbr);
            visited[nbr] = true;
          }
        }
      }
      visited[start] = false; // assuming no cycle, so vertex cannot reach itself
      return visited;
    }

    std::optional<int> getIndex(ProcedureId procedureId) const {
      // if the procedureId is never registered, then we return a NOT_FOUND index
      if (map_.find(procedureId) == map_.end()) {
        return std::nullopt;
      }
      return std::optional<int>{map_.at(procedureId)};
    }
    int getIndex(ProcedureId procedureId) {
      auto it = map_.find(procedureId);
      if (it == map_.end()) {
        auto ret = nextIndex_++;
        procedureIds_[ret] = procedureId;
        map_.insert({procedureId, ret});
        return ret;
      } else {
        return it->second;
      }
    }
    ProcedureId getProcedureId(int index) const {
      return procedureIds_[index];
    }

    int nextIndex_ = 0;
    unsigned numProcedures_;
    std::unordered_map<ProcedureId, int> map_; // assigns each procedure an vertex number
    std::vector<ProcedureId> procedureIds_;
    std::vector<std::unordered_set<int>> adjList_;
    std::vector<std::unordered_set<int>> transposeAdjList_;
  };
}
