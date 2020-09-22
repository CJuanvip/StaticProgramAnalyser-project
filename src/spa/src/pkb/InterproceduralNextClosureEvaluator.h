#pragma once
#include <RealtimeEvaluator.h>
#include <ControlFlowGraph.h>
#include <memory>
#include <PKBTable.h>
#include <CallGraph.h>
namespace pkb {
class PKBTableManager;
class InterproceduralNextClosureEvaluator : public RealtimeEvaluator<unsigned, unsigned> {
public:
  InterproceduralNextClosureEvaluator() : numStatements_(0), cfg_(nullptr) { }
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
