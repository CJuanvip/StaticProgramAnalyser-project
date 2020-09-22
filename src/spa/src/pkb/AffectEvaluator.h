#pragma once
#include <RealtimeEvaluator.h>
#include <ControlFlowGraph.h>
#include <memory>
#include <PKBTable.h>
namespace pkb {
class PKBTableManager;
class AffectEvaluator : public RealtimeEvaluator<unsigned, unsigned> {
public:
  AffectEvaluator() { }
  void initialize(PKBTableManager&, ControlFlowGraph&);
  bool select(PKBTable<unsigned, unsigned>& table) override;
  bool filterLeft(const unsigned& left, PKBTable<unsigned, unsigned>&) override;
  bool filterRight(const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  bool contains(const unsigned& left, const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  void reset();
  const std::vector<std::vector<bool>>& getAffect();
private:
  void extract(PKBTable<unsigned, unsigned>& table);
  unsigned numStatements_;
  ControlFlowGraph* cfg_;
  PKBTableManager* mgr_;
};
}
