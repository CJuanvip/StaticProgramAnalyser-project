#include <NextClosureEvaluator.h>
#include <PKBTableManager.h>
#include <CfgVisitor.h>
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
#include <SPAAssert.h>
namespace pkb {
  namespace {
  class NextClosureExtractor : public ast::CfgVisitor {
  public:
    NextClosureExtractor() = delete;
    NextClosureExtractor(const pkb::ControlFlowGraph& cfg)
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
        transitiveClosure(i, i, true);
      }
    }
  private:
    void transitiveClosure(unsigned src, unsigned reachable, bool isSelfLoop) {
      transitiveClosures_[src][reachable] = (!isSelfLoop) & true;
      // Find all the vertices reachable
      for (const auto dest: adjList_[reachable]) {
          if (!transitiveClosures_[src][dest]) {
            transitiveClosure(src, dest, false);
          }
      }
    }
    std::vector<std::vector<bool>> transitiveClosures_;
  };
  }// namespace anonymous
void NextClosureEvaluator::initialize(PKBTableManager& mgr, ControlFlowGraph& cfg) {
  numStatements_ = mgr.getStatementsTable().count();
  cfg_ = &cfg;
  SPA_ASSERT(numStatements_ == cfg_->getNumStatements());
}
bool NextClosureEvaluator::select(PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool NextClosureEvaluator::filterLeft(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool NextClosureEvaluator::filterRight(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool NextClosureEvaluator::contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
void NextClosureEvaluator::reset() {
}
void NextClosureEvaluator::extract(PKBTable<unsigned, unsigned>& table) {
  SPA_ASSERT(numStatements_ != 0);
  SPA_ASSERT(cfg_ != nullptr);
  NextClosureExtractor visitor(*cfg_);
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
