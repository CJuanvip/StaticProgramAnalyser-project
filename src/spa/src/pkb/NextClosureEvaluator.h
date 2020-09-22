#pragma once
#include <RealtimeEvaluator.h>
#include <ControlFlowGraph.h>
#include <memory>
#include <PKBTable.h>
namespace pkb {
class PKBTableManager;
class NextClosureEvaluator : public RealtimeEvaluator<unsigned, unsigned> {
public:
  NextClosureEvaluator() : numStatements_(0), cfg_(nullptr) { }
  void initialize(PKBTableManager&, ControlFlowGraph&);
  bool select(PKBTable<unsigned, unsigned>& table) override;
  bool filterLeft(const unsigned& left, PKBTable<unsigned, unsigned>&) override;
  bool filterRight(const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  bool contains(const unsigned& left, const unsigned& right, PKBTable<unsigned, unsigned>&) override;
  void reset();
private:
  void extract(PKBTable<unsigned, unsigned>& table);
  unsigned numStatements_;
  ControlFlowGraph* cfg_;
};
}
