#include "Evaluator.h"
#include "Visitor.h"
#include <SPAAssert.h>

namespace query_eval {
  /**
   * =================================================================
   * Stage 4: Clause Evaluation
   * Methods for With clauses
   * =================================================================
   */

  void Evaluator::sortWiths(const std::vector<With> &withs) {
    for (const auto &with : withs) {
      const auto&[lhs, rhs] = with;
      if (std::holds_alternative<AttrRef>(lhs) && std::holds_alternative<AttrRef>(rhs)) {
        twoSynonymWiths_.emplace_back(with);
      } else if (std::holds_alternative<AttrRef>(lhs) || std::holds_alternative<AttrRef>(rhs)) {
        oneSynonymWiths_.emplace_back(with);
      } else {
        booleanWiths_.emplace_back(with);
      }
    }
  }

  bool Evaluator::processBooleanWith(const With &with) {
    const auto&[lhs, rhs] = with;
    SPA_ASSERT(!std::holds_alternative<AttrRef>(lhs));
    SPA_ASSERT(!std::holds_alternative<AttrRef>(rhs));
    // Should be the same type
    SPA_ASSERT(lhs.index() == rhs.index());
    return lhs == rhs;
  }

  bool Evaluator::processOneSynonymWith(const With &with) {
    const auto&[lhs, rhs] = with;
    if (std::holds_alternative<AttrRef>(lhs)) {
      SPA_ASSERT(!std::holds_alternative<AttrRef>(rhs));
      SPA_ASSERT(std::holds_alternative<AttrRef>(lhs));

      const AttrRef &leftSynonym = std::get<AttrRef>(lhs);
      SchemaType leftIndex = getSynonymIndex(leftSynonym).value();

      std::optional<CellType> nameIdOpt = std::visit(visitor {
        [&](const AttrRef &) { return std::optional<CellType>{}; },
        [&](const Int &i) { return cache_.getNameId(i.value); },
        [&](const std::string &s) { return cache_.getNameId(s); }
      }, rhs);
      // LHS will have no result anyway
      if (!nameIdOpt.has_value()) return false;
      if (!cache_.contains(entityType_[leftIndex], leftSynonym.attribute, nameIdOpt.value())) {
        return false;
      }

      // Generates a 1 by 1 clause table to contraint that synonym
      return emplaceTable(nameIdOpt.value(), leftIndex);
    } else {
      SPA_ASSERT(std::holds_alternative<AttrRef>(rhs));
      SPA_ASSERT(!std::holds_alternative<AttrRef>(lhs));
      const AttrRef &rightSynonym = std::get<AttrRef>(rhs);
      SchemaType rightIndex = getSynonymIndex(rightSynonym).value();

      std::optional<CellType> nameIdOpt = std::visit(visitor {
        [&](const AttrRef &) { return std::optional<CellType>{}; },
        [&](const Int &i) { return cache_.getNameId(i.value); },
        [&](const std::string &s) { return cache_.getNameId(s); }
      }, lhs);

      // LHS will have no result anyway
      if (!nameIdOpt.has_value()) return false;
      if (!cache_.contains(entityType_[rightIndex], rightSynonym.attribute, nameIdOpt.value())) {
        return false;
      }

      // Generates a 1 by 1 clause table to constraint that synonym
      return emplaceTable(nameIdOpt.value(), rightIndex);
    }
  }

  bool Evaluator::processTwoSynonymWith(const With &with) {
    const auto&[lhs, rhs] = with;
    const AttrRef &leftSynonym = std::get<AttrRef>(lhs);
    const AttrRef &rightSynonym = std::get<AttrRef>(rhs);

    SchemaType leftIndex = getSynonymIndex(leftSynonym).value();
    SchemaType rightIndex = getSynonymIndex(rightSynonym).value();

    if (synonymPredetermined_.count(leftIndex) != 0) {
      With clonedWith = with;
      clonedWith.first = cache_.getName(synonymPredetermined_.at(leftIndex));
      return processOneSynonymWith(clonedWith);
    }
    if (synonymPredetermined_.count(rightIndex) != 0) {
      With clonedWith = with;
      clonedWith.second = cache_.getName(synonymPredetermined_.at(rightIndex));
      return processOneSynonymWith(clonedWith);
    }

    EntityId baseLeftEntity = entityType_[getSynonymIndex(leftSynonym.synonym).value()];
    EntityId baseRightEntity = entityType_[getSynonymIndex(rightSynonym.synonym).value()];

    ResultTable intersection =
      cache_.select(baseLeftEntity, leftSynonym.attribute) *
      cache_.select(baseRightEntity, rightSynonym.attribute);

    if (intersection.size() == 0) return false; // This clause is definitely false

    // Same attribute , just need to check if there exist any rows
    if (leftIndex == rightIndex) return true;

    // Duplicate the intersection data
    std::vector<RowType> data = intersection.data();
    for (auto &row : data) {
      row.emplace_back(row[0]);
    }
    return emplaceTable(
      ResultTable(data).reschema({leftIndex, rightIndex})
    );
  }
};
