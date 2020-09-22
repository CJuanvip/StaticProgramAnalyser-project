#include <InterproceduralNextExtractor.h>
#include <CfgVisitor.h>
namespace ast {
  class InterproceduralNextVisitor: public CfgVisitor {
  public:
    InterproceduralNextVisitor(const pkb::ControlFlowGraph& cfg, pkb::PKBTableManager& mgr)
    : CfgVisitor(cfg)
    , interproceduralNextTable_(mgr.getInterproceduralNext()) { }
    void visit() override {
      for (unsigned src = 0; src < adjList_.size(); ++src) {
        if (branchLinks_[src] != 0) {
          interproceduralNextTable_.insert(src, branchLinks_[src]);
        } else {
          for (const auto& dest: adjList_[src]) {
            if (dest != 0) { 
              interproceduralNextTable_.insert(src, dest);
            } else {
              // check for backwardLinks
              if (backwardLinks_[src].size() == 0) {
                continue;
              }
              const auto dests = getDests(src);
              for (const auto& dest: dests) {
                interproceduralNextTable_.insert(src, dest);
              }
            }
          }
        }
      }
    }
  private:
    pkb::PKBTable<unsigned, unsigned>& interproceduralNextTable_;
  };
  void extractInterproceduralNext(const pkb::ControlFlowGraph& cfg, pkb::PKBTableManager& mgr) {
    InterproceduralNextVisitor visitor(cfg, mgr);
    visitor.visit();
  }
}