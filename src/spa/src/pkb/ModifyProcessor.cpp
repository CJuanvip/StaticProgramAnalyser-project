#include <ModifyProcessor.h>
#include <PKBTableManager.h>
namespace pkb {
  void ModifyProcessor::postProcess(PKBTableManager& mgr, CallGraph& callGraph) {
    auto topologicalOrder = callGraph.getTopologicalOrder();
    for (auto it = topologicalOrder.rbegin(); it != topologicalOrder.rend(); ++it) {
      auto callerIds = callGraph.getCallerIds(*it);
      for(auto callerId: callerIds) {
        transferTransitivity(callerId, *it);
      }
    }
    transferToTables(mgr);
  }
  void ModifyProcessor::transferToTables(PKBTableManager& mgr) {
    auto& modifyEntity = mgr.getModifyEntity();
    for (auto& p: map_) {
      auto procedureId = p.first;
      for (auto& q: p.second) {
        auto statementId = q.first;
        auto& variableIds= q.second;
        for(auto varId: variableIds) {
          modifyEntity.insert(statementId, varId);
          modifyEntity.insert(procedureId, varId);
        }
      }
    }
  }
};