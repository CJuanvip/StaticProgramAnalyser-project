#include <AffectClosureEvaluator.h>
#include <PKBTableManager.h>
#include <CfgVisitor.h>
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
#include <EntityId.h>
namespace pkb {
  namespace {
  class AffectClosureExtractor {
  public:
    AffectClosureExtractor(pkb::PKBTableManager& mgr) : mgr_(mgr), affectsTable_(mgr.getAffect()) {
      const auto size = mgr_.getStatementsTable().count() + 1;
      affectsClosure_.assign(size, std::vector<bool>(size, false));
    }
    const std::vector<std::vector<bool>> getAffectsClosure() const {
      return affectsClosure_;
    }
    void visit() {
      for (auto i : mgr_.getStatementsTable().filterRight(common::EntityId::Assign)) {
        transitiveClosure(i, i);
      }
    }
  private:
    void transitiveClosure(unsigned src, unsigned reachable) {
      // Find all the vertices reachable
      for (const auto dest: affectsTable_.filterLeft(reachable)) { 
        if (!affectsClosure_[src][dest]) {
          affectsClosure_[src][dest] = true;
          transitiveClosure(src, dest); 
        }
      }
    }
    pkb::PKBTableManager& mgr_;
    std::vector<std::vector<bool>> affectsClosure_;
    pkb::RealtimePKBTable<unsigned, unsigned>& affectsTable_;
  };
  }// namespace anonymous
void AffectClosureEvaluator::initialize(PKBTableManager& mgr) {
  numStatements_ = mgr.getStatementsTable().count();
  mgr_ = &mgr;
}
bool AffectClosureEvaluator::select(PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectClosureEvaluator::filterLeft(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectClosureEvaluator::filterRight(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectClosureEvaluator::contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
void AffectClosureEvaluator::reset() {
}
void AffectClosureEvaluator::extract(PKBTable<unsigned, unsigned>& table) {
  AffectClosureExtractor visitor(*mgr_);
  visitor.visit();
  const auto& affectsClosure = visitor.getAffectsClosure();
  for (size_t i = 1; i <= numStatements_; ++i) {
    for (size_t j = 1; j <= numStatements_; ++j) {
      if (affectsClosure[i][j]) {
        table.insert(i,j);
      }
    }
  }
}
}
