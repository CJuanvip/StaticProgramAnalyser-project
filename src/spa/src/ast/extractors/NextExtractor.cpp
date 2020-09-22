#include <NextExtractor.h>
#include <CfgVisitor.h>
#include <PKBTableManager.h>
namespace ast {
  namespace {
  class NextVisitor : public CfgVisitor {
  public:
    NextVisitor(const pkb::ControlFlowGraph& cfg, pkb::PKBTableManager& mgr)
    : CfgVisitor(cfg)
    , nextTable_(mgr.getNext()) { }
    void visit() override {
      for (unsigned src = 0; src < adjList_.size(); ++src) {
        for (const auto& dest: adjList_[src]) {
          if (dest != 0) { // Unlikely
            nextTable_.insert(src, dest);
          }
        }
      }
    }
  private:
    pkb::PKBTable<unsigned, unsigned>& nextTable_;
  };
  } // namespace anonymous
  void extractNext(const pkb::ControlFlowGraph& cfg, pkb::PKBTableManager& mgr) {
    NextVisitor visitor(cfg, mgr);
    visitor.visit();
  }
}