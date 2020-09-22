#pragma once
#include <RealtimeEvaluator.h>
#include <ControlFlowGraph.h>
#include <memory>
#include <PKBTable.h>
#include <CallGraph.h>
namespace pkb {
class PKBTableManager;
class InterproceduralAffectEvaluator : public RealtimeEvaluator<unsigned, unsigned> {
public:
  InterproceduralAffectEvaluator()  { }
  void initialize(PKBTableManager&, ControlFlowGraph&, CallGraph&);
  bool select(PKBTable<unsigned, unsigned>& table) override;
  bool filterLeft(const unsigned& left, PKBTable<unsigned, unsigned>&) override;
  bool filterRight(const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  bool contains(const unsigned& left, const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  void reset();
private:
  void extract(PKBTable<unsigned, unsigned>& table);
  pkb::PKBTableManager* mgr_;
  pkb::ControlFlowGraph* cfg_;
  pkb::CallGraph* callGraph_;
  unsigned numStatements_;
};
}
