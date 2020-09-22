#include <InterproceduralNextClosureEvaluator.h>
#include <PKBTableManager.h>
#include <CfgVisitor.h>
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
#include <SPAAssert.h>
namespace pkb {
  namespace {
  class InterproceduralNextClosureExtractor : public ast::CfgVisitor {
  public:
    InterproceduralNextClosureExtractor() = delete;
    InterproceduralNextClosureExtractor(const pkb::ControlFlowGraph& cfg)
    : ast::CfgVisitor(cfg) {
      auto size = cfg.getNumStatements() + 1; // Here, we add 1 so that index equals statement id
      transitiveClosures_.resize(size);
      for(auto& v: transitiveClosures_) {
        v.resize(size);
      }
    }
    const std::vector<std::vector<bool>> getTransitiveClosures() const {
      return transitiveClosures_;
    }
    void visit() override {
      for (size_t i = 1; i < transitiveClosures_.size(); ++i) {
        std::vector<unsigned> callStack;
        transitiveClosure(i, i, true, callStack);
      }
    }
  private:
    void transitiveClosure(unsigned src, unsigned reachable, bool isSelfLoop, std::vector<unsigned>& callStack) {
      transitiveClosures_[src][reachable] = (!isSelfLoop) & true;
      if (branchLinks_[reachable] != 0) {
        callStack.push_back(reachable);
        transitiveClosure(src, branchLinks_[reachable], false, callStack);
        callStack.pop_back();
      }
      for (const auto dest: adjList_[reachable]) {
        if (dest == 0) {
          if (callStack.empty()) {
            const auto dests = getDests(reachable);
            for (const auto d: dests) {
              if (!transitiveClosures_[src][d]) {
                transitiveClosure(src, d, false, callStack);
              }
            }
          }
        } else {
          if (!transitiveClosures_[src][dest]) {
            transitiveClosure(src, dest, false, callStack);
          }
        }
      }
    }
    std::vector<std::vector<bool>> transitiveClosures_;
  };
  }// namespace anonymous
void InterproceduralNextClosureEvaluator::initialize(PKBTableManager& mgr, ControlFlowGraph& cfg) {
  numStatements_ = mgr.getStatementsTable().count();
  cfg_ = &cfg;
  SPA_ASSERT(numStatements_ == cfg_->getNumStatements());
}
bool InterproceduralNextClosureEvaluator::select(PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralNextClosureEvaluator::filterLeft(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralNextClosureEvaluator::filterRight(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool InterproceduralNextClosureEvaluator::contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
void InterproceduralNextClosureEvaluator::reset() {
}
void InterproceduralNextClosureEvaluator::extract(PKBTable<unsigned, unsigned>& table) {
  SPA_ASSERT(numStatements_ != 0);
  SPA_ASSERT(cfg_ != nullptr);
  InterproceduralNextClosureExtractor visitor(*cfg_);
  visitor.visit();
  const auto& transitiveClosures = visitor.getTransitiveClosures();
  for (size_t i = 1; i <= numStatements_; ++i) {
    for (size_t j = 1; j <= numStatements_; ++j) {
      if (transitiveClosures[i][j]) {
        table.insert(i,j);
      }
    }
  }
}
}
