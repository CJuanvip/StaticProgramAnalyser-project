#include <AffectEvaluator.h>
#include <PKBTableManager.h>
#include <CfgVisitor.h>
#include <ControlFlowGraph.h>
#include <PKBTableManager.h>
#include <SPAAssert.h>
namespace pkb {
  namespace {
  // a stateless extractor
  class AffectExtractor: public ast::CfgVisitor {
  public:
    AffectExtractor() = delete;
    AffectExtractor(const ControlFlowGraph& cfg, pkb::PKBTableManager& mgr)
    : ast::CfgVisitor(cfg), mgr_(mgr) {
      // do something here, probably initialize DS
      auto size = cfg.getNumStatements() + 1; // Here, we add 1 so that index equals statement id
      affect_.resize(size);
      visited_.resize(size);
      for(auto& v: affect_) {
        v.resize(size);
      }
      const auto& statementTable = mgr.getStatementsTable();
      const auto& assignIds = statementTable.filterRight(common::EntityId::Assign);
      auto& assignLhsTable = mgr.getAssignLhs();
      auto& nameTypesTable = mgr.getNameTypesTable();
      for(const auto assignId : assignIds) {
        const auto& variableAndConstIds = assignLhsTable.filterLeft(assignId);
        for(const auto id: variableAndConstIds) {
          if (nameTypesTable.contains(id, common::EntityId::Variable)) {
            registerVariable(id);
          }
        }
      }
      numVariables_ = indexToVariableIdMap_.size();
      statementIdToModifiedIndexes_.resize(size);
      for (size_t i = 1; i < statementIdToModifiedIndexes_.size(); ++i) {
        auto& v = statementIdToModifiedIndexes_[i];
        const auto& filtered = statementTable.filterLeft(i);
        SPA_ASSERT(filtered.size() == 1);
        const auto& statementType = *filtered.begin();
        if (statementType == common::EntityId::Assign
         || statementType == common::EntityId::Call
         || statementType == common::EntityId::Read) {
          setModifiedIndexes(i, v);
        }
      }
      statementIdToUsedIndexes_.resize(size);
      for (size_t i = 1; i < statementIdToUsedIndexes_.size(); ++i) {
        auto& v = statementIdToUsedIndexes_[i];
        if (assignIds.find(i) != assignIds.end()) {
          setUsedIndexes(i, v);
        }
      }
    }
    void visit() override {
      const auto& assignIds = mgr_.getStatementsTable().filterRight(common::EntityId::Assign);
      for (const auto assignId : assignIds) {
        computeAffect(assignId);
      }
    }
    const std::vector<std::vector<bool>>& getAffect() {
      return affect_;
    }
  private:
    void computeAffect(unsigned src) {
      unsigned modified;
      initializeStatusWithModify(src, modified);
      for (const auto dest: adjList_[src]) {
        computeAffect(src, dest, modified);
      }
    }

    void computeAffect(unsigned src, unsigned dest, unsigned modified) {
      if (visited_[dest] || dest == 0) {
        return;
      }
      bool isAffect = processStatusWithUses(dest, modified);
      if (isAffect) {
        affect_[src][dest] = true;
      }

      auto shouldContinue = processStatusWithModifies(dest, modified);
      if (!shouldContinue) {
        return;
      }
      for (const auto next: adjList_[dest]) {
        visited_[dest] = true;
        computeAffect(src, next, modified);
        visited_[dest] = false;
      }

    }

    bool processStatusWithUses(unsigned statementId, unsigned modified) {
      const auto& usesSet = statementIdToUsedIndexes_[statementId];
      for (const auto& index: usesSet) {
        if (index == modified) {
          return true;
        }
      }
      return false;
    }

    bool processStatusWithModifies(unsigned statementId, unsigned modified) {
      const auto& modifiesSet = statementIdToModifiedIndexes_[statementId];
      if (modifiesSet.find(modified) != modifiesSet.end()) {
        return false;
      } else {
        return true;
      }
    }

    void initializeStatusWithModify(unsigned statementId, unsigned& modified) {
      const auto& modifiesSet = statementIdToModifiedIndexes_[statementId];
      SPA_ASSERT(modifiesSet.size() == 1);
      modified = *modifiesSet.cbegin();
    }

    void registerVariable(unsigned variableId) {
      if (variableIdToIndexMap_.find(variableId) == variableIdToIndexMap_.end()) {
        auto index = indexToVariableIdMap_.size();
        indexToVariableIdMap_.push_back(variableId);
        variableIdToIndexMap_.insert({variableId, index});
      }
    }
    void setModifiedIndexes(unsigned statementId, std::unordered_set<unsigned>& modifiedSet) {
      const auto& modifyTable = mgr_.getModifyEntity();
      const auto& modifiedVariableIds = modifyTable.filterLeft(statementId);
      for (const auto varId : modifiedVariableIds) {
        if (variableIdToIndexMap_.find(varId) != variableIdToIndexMap_.end()) {
          modifiedSet.insert(variableIdToIndexMap_[varId]);
        }
      }
    }
    void setUsedIndexes(unsigned statementId, std::unordered_set<unsigned>& usedSet) {
      const auto& usedTable = mgr_.getUsesEntityVariableTable();
      const auto& usedVariableIds = usedTable.filterLeft(statementId);
      for (const auto varId : usedVariableIds) {
        if (variableIdToIndexMap_.find(varId) != variableIdToIndexMap_.end()) {
          usedSet.insert(variableIdToIndexMap_[varId]);
        }
      }
    }
    std::vector<std::vector<bool>> affect_;
    // those only appears in assignment will have an index; index is 0-based

    std::vector<unsigned> indexToVariableIdMap_;
    std::unordered_map<unsigned, unsigned> variableIdToIndexMap_;

    // Only those variable registered above will be added here
    std::vector<std::unordered_set<unsigned>> statementIdToModifiedIndexes_;
    std::vector<std::unordered_set<unsigned>> statementIdToUsedIndexes_;
    unsigned numVariables_;
    std::vector<bool> visited_;
    pkb::PKBTableManager& mgr_;
  };
  }// namespace anonymous
void AffectEvaluator::initialize(PKBTableManager& mgr, ControlFlowGraph& cfg) {
  numStatements_ = mgr.getStatementsTable().count();
  mgr_ = &mgr;
  cfg_ = &cfg;
}
bool AffectEvaluator::select(PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectEvaluator::filterLeft(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectEvaluator::filterRight(const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
bool AffectEvaluator::contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
  extract(table);
  return true;
}
void AffectEvaluator::reset() {
}
void AffectEvaluator::extract(PKBTable<unsigned, unsigned>& table) {
  AffectExtractor visitor(*cfg_, *mgr_);
  visitor.visit();
  const auto& affect = visitor.getAffect();
  for (size_t i = 1; i <= numStatements_; ++i) {
    for (size_t j = 1; j <= numStatements_; ++j) {
      if (affect[i][j]) {
        table.insert(i,j);
      }
    }
  }
}
}
