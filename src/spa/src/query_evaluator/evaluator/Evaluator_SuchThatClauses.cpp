#include "Evaluator.h"
#include <SPAAssert.h>
namespace query_eval {
  /**
   * =================================================================
   * Stage 4: Clause Evaluation
   * Methods for SuchThat clauses
   * =================================================================
   */

  void Evaluator::sortSuchThats(const std::vector<SuchThat> &suchThats) {
    for (auto &suchThat:suchThats) {
      SuchThatClause clause(suchThat);
      SPA_ASSERT(clause.isValid());
      auto &[relation, left, right] = clause;
      if (left.isEntity() && right.isEntity()) {
        twoSynonymSuchThats_.emplace_back(clause);
      } else if (left.isEntity() || right.isEntity()) {
        oneSynonymSuchThats_.emplace_back(clause);
      } else {
        booleanSuchThats_.emplace_back(clause);
      }
    }
  }



  // Such thats with no entities can be evaluated to TRUE/FALSE directly
  bool Evaluator::processBooleanSuchThat(const SuchThatClause &clause) {
    const Relation &relation = clause.relation;
    const Argument &left = clause.leftArg, &right = clause.rightArg;
    if (left.isWildcard() && right.isWildcard()) {
      return cache_.hasWildWild(relation);
    } else if (left.isWildcard()) {
      auto rightNameId = cache_.getNameId(right).value();
      return cache_.hasWildRight(relation, rightNameId);
    } else if (right.isWildcard()) {
      auto leftNameId = cache_.getNameId(left).value();
      return cache_.hasLeftWild(relation, leftNameId);
    } else {
      // Both sides are constants
      auto leftNameId = cache_.getNameId(left).value();
      auto rightNameId = cache_.getNameId(right).value();
      return cache_.contains(relation, leftNameId, rightNameId);
    }
  }

  // Relations with one entity effectively prunes the possible results for that entity only,
  bool Evaluator::processOneSynonymSuchThat(const SuchThatClause &clause) {
    const Relation &relation = clause.relation;
    const Argument &left = clause.leftArg, &right = clause.rightArg;
    if (left.isEntity()) {
      SchemaType leftEntityIndex = getSynonymIndex(left).value();
      EntityId leftType = entityType_.at(leftEntityIndex);
      if (right.isWildcard()) {
        return emplaceTable(
          cache_.selectLeft(relation, leftType)
          .reschema({leftEntityIndex})
        );
      } else {
        SPA_ASSERT(right.isConstant());
        auto rightNameId = cache_.getNameId(right).value();
        return emplaceTable(
          cache_.filterRight(relation, leftType, rightNameId)
          .reschema({leftEntityIndex})
        );
      }
    } else {
      SPA_ASSERT(right.isEntity());
      SchemaType rightEntityIndex = getSynonymIndex(right).value();
      EntityId rightType = entityType_.at(rightEntityIndex);

      if (left.isWildcard()) {
        return emplaceTable(
          cache_.selectRight(relation, rightType)
          .reschema({rightEntityIndex})
        );
      } else {
        SPA_ASSERT(left.isConstant());
        auto leftNameId = cache_.getNameId(left).value();
        return emplaceTable(
          cache_.filterLeft(relation, leftNameId, rightType)
          .reschema({rightEntityIndex})
        );
      }
    }
    return true;
  }

  // Such thats with 2 entities nessitates a table join
  // EDGE case: the 2 entities are the same --> filter
  bool Evaluator::processTwoSynonymSuchThat(const SuchThatClause &clause) {
    const Relation &relation = clause.relation;
    const Argument &left = clause.leftArg, &right = clause.rightArg;
    SchemaType leftEntityIndex = getSynonymIndex(left).value(), rightEntityIndex = getSynonymIndex(right).value();
    EntityId leftType = entityType_.at(leftEntityIndex), rightType = entityType_.at(rightEntityIndex);
    if (synonymPredetermined_.count(leftEntityIndex) != 0) {
      SuchThatClause clonedClause = clause;
      clonedClause.leftArg = cache_.getName(synonymPredetermined_.at(leftEntityIndex));
      if (!clonedClause.leftArg.isNumericConstant()) {
        clonedClause.leftArg = "\"" + (std::string) clonedClause.leftArg + "\"";
      }
      return processOneSynonymSuchThat(clonedClause);
    }
    if (synonymPredetermined_.count(rightEntityIndex) != 0) {
      SuchThatClause clonedClause = clause;
      clonedClause.rightArg = cache_.getName(synonymPredetermined_.at(rightEntityIndex));
      if (!clonedClause.rightArg.isNumericConstant()) {
        clonedClause.rightArg = "\"" + (std::string) clonedClause.rightArg + "\"";
      }
      return processOneSynonymSuchThat(clonedClause);
    }
    if (leftEntityIndex != rightEntityIndex) {
      return emplaceTable(
        cache_.select(relation, leftType, rightType)
          .reschema({leftEntityIndex, rightEntityIndex})
      );
    } else {
      return emplaceTable(
        cache_.selectSame(relation, leftType)
          .reschema({leftEntityIndex})
      );
    }
    return true;
  }
};
