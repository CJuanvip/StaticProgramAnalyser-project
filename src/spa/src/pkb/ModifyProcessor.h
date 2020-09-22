#pragma once
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <CallGraph.h>
namespace pkb {
  class PKBTableManager;
  class ModifyProcessor {
  public: 
    void addModifies(unsigned procedureId, std::vector<unsigned>& statementIds, 
                     unsigned variableId) {
      for (auto id: statementIds) {
        map_[procedureId][id].insert(variableId);
      }
    }
    
    void addCallingRelationship(unsigned callerId, unsigned calleeId, std::vector<unsigned>& statementIds) {
      for(auto statementId: statementIds) {
        calls_[callerId][calleeId].insert(statementId);
      }
    }

    // precondition is no cyclic call
    // We need to identify topological order of the calling
    // and we enrich our record in the reverse order
    void postProcess(PKBTableManager&, CallGraph&);
  private:
    void transferTransitivity(unsigned callerId, unsigned calleeId) {
      auto& from = map_[calleeId];
      std::unordered_set<unsigned> modifiedVarIds;
      for(auto p: from) {
        for(auto& varId : from.at(p.first)) {
          modifiedVarIds.insert(varId);
        }
      }
      auto& to = map_[callerId];
      for (auto statementId: calls_[callerId][calleeId]) {
        for (auto varId: modifiedVarIds) {
          to[statementId].insert(varId);
        }
      }
    }
    void transferToTables(PKBTableManager&);
    // proc_id -> stmt_id -> var_id
    std::unordered_map<unsigned, std::unordered_map<unsigned, std::unordered_set<unsigned>>> map_;
    std::unordered_map<unsigned, std::unordered_map<unsigned, std::unordered_set<unsigned>>> calls_;
  };
}