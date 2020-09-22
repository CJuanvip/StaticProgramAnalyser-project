#include "Evaluator.h"
#include "Visitor.h"
#include <unordered_set>

namespace query_eval {
 /**
   * =================================================================
   * Stage 3: Early termination
   * =================================================================
   */
  bool Evaluator::canEarlyTerminate(const Query& query) const {
    
    /**
     * (1) For constant int/strings that appear in such that clauses,
     * if they do not appear anywhere in PKB --> definitely no result.
     */
    auto checkStmtRef = [&](const StmtRef& ref) {
      if (std::holds_alternative<Int>(ref)) {
        return cache_.getNameId(std::get<Int>(ref).value).has_value();
      }
      return true;
    };

    auto checkEntRef = [&](const EntRef& ref) {
      if (std::holds_alternative<Var>(ref)) {
        return cache_.getNameId(std::get<Var>(ref).name).has_value();
      }
      return true;
    };
    for (const auto &suchThat: query.suchThats) {
      bool allPresent = std::visit(visitor{
        [&](const StmtStmtRelation& clause) {
          return checkStmtRef(clause.left) && checkStmtRef(clause.right);
        },
        [&](const StmtEntRelation& clause) {
          return checkStmtRef(clause.left) && checkEntRef(clause.right);
        },
        [&](const EntEntRelation& clause) {
          return checkEntRef(clause.left) && checkEntRef(clause.right);
        },
        [&](const StmtEntEntRelation& clause) {
          return checkEntRef(clause.right) && std::visit(visitor{
            [&](const StmtRef& ref) { return checkStmtRef(ref); },
            [&](const EntRef& ref) { return checkEntRef(ref); }
          }, clause.left);
        }
      }, suchThat);
      if (!allPresent) return true;
    }

    for (const auto &pattern: query.patterns) {
      bool allPresent = std::visit(visitor{
        [&](const BinaryPattern& p) {
          if (std::holds_alternative<Var>(p.left)) {
            return cache_.getNameId(std::get<Var>(p.left).name).has_value();
          }
          return true;
        },
        [&](const TernaryPattern& p) {
          if (std::holds_alternative<Var>(p.left)) {
            return cache_.getNameId(std::get<Var>(p.left).name).has_value();
          }
          return true;
        }
      }, pattern);
      if (!allPresent) return true;
    }

    /**
     * (2) For the following relations, if the LHS and RHS are the same type,
     * there will be no result.
     */
    const std::unordered_set<RelationId> sameEntityNoResult = {
      RelationId::Follows,
      RelationId::FollowsT,
      RelationId::Parent,
      RelationId::ParentT,
      RelationId::Calls,
      RelationId::CallsT,
      RelationId::Next,
      RelationId::NextBip,
    };
    for (const auto& suchThat : query.suchThats) {
      // So far only for StmtStmtRelation
      if (std::holds_alternative<StmtStmtRelation>(suchThat)) {
        const StmtStmtRelation &clause = std::get<StmtStmtRelation>(suchThat);
        if (sameEntityNoResult.count(clause.type) == 1 &&
          std::holds_alternative<Synonym>(clause.left) &&
          std::holds_alternative<Synonym>(clause.right) &&
          std::get<Synonym>(clause.left) == std::get<Synonym>(clause.right)) {
          return false;
        }
      }
    }
    return false;
  }

};
