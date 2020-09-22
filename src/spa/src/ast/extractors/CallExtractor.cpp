#include <CallExtractor.h>
namespace ast {
  void extractCall(const pkb::CallGraph& callGraph, pkb::PKBTableManager& mgr) {
    auto& callTable = mgr.getCall();
    const auto directCalls = callGraph.getDirectCallerCalleePairs();
    for(const auto& [callerId, calleeId]: directCalls) {
      callTable.insert(callerId, calleeId);
    }

    auto& callClosureTable = mgr.getCallClosure();
    const auto allCalls = callGraph.getAllCallerCalleePairs();
    for(const auto& [callerId, calleeId]: allCalls) {
      callClosureTable.insert(callerId, calleeId);
    }
  }
}