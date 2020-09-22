#include "Evaluator.h"
#include "Visitor.h"
#include <SPAAssert.h>

namespace query_eval {
 /**
   * =================================================================
   * Stage 2: Synonym normalization
   * =================================================================
   */
  bool Evaluator::normalizeDeclarations(const std::vector<Declaration> &declarations) {
    for (auto &[entityId, synonym]: declarations) {
      size_t index = entityType_.size();
      entityType_.emplace_back(entityId);
      // Synonym should not already exist
      if (synonymToIndex_.count(synonym) != 0) {
        return false;
      }
      synonymToIndex_[synonym] = index;
    }
    return true;
  }

  SchemaType Evaluator::normalizeAttrRef(const AttrRef &attrRef) {
    // Already normalized
    if (getSynonymIndex(attrRef).has_value()) {
      return getSynonymIndex(attrRef).value();
    }
    // Base index
    std::optional<SchemaType> index = getSynonymIndex(attrRef.synonym);
    // Must be declared
    SPA_ASSERT(index.has_value());
    // Optimization, check if entity has result
    SchemaType synonymIndex = index.value();
    EntityId entityType = entityType_.at(synonymIndex);
    if (TYPE_ID_ATTRIBUTE_EQUAL.at(entityType).count(attrRef.attribute.value()) != 1) {
      // Not equivalent to base entity
      SchemaType newSynonymIndex = entityType_.size();
      if (entityType == EntityId::Read && attrRef.attribute.value() == AttrNameId::VarName) {
        clauseTables_.emplace_back(
          cache_.select(RelationId::ReadVar, EntityId::Read, EntityId::Variable)
          .reschema({synonymIndex, newSynonymIndex})
        );
        entityType_.emplace_back(EntityId::Variable);
      } else if (entityType == EntityId::Print && attrRef.attribute.value() == AttrNameId::VarName) {
        clauseTables_.emplace_back(
          cache_.select(RelationId::PrintVar, EntityId::Print, EntityId::Variable)
          .reschema({synonymIndex, newSynonymIndex})
        );
        entityType_.emplace_back(EntityId::Variable);
      } else {
        SPA_ASSERT(entityType == EntityId::Call);
        SPA_ASSERT(attrRef.attribute.value() == AttrNameId::ProcName);
        clauseTables_.emplace_back(
          cache_.select(RelationId::CallProc, EntityId::Call, EntityId::Procedure)
          .reschema({synonymIndex, newSynonymIndex})
        );
        entityType_.emplace_back(EntityId::Procedure);
      }
      synonymToIndex_[attrRef] = newSynonymIndex;
      synonymIndex = newSynonymIndex;
    } else {
      // Equivalent, just assign the same index
      synonymToIndex_[attrRef] = synonymIndex;
    }
    return synonymToIndex_[attrRef];
  }

  bool Evaluator::normalizeResultList(const std::vector<AttrRef> &resultList) {
    resultList_.clear();
    for (auto &attrRef : resultList) {
      // Check that the base synonym has been declared
      // Should be done in hasSemanticError
      // if (getSynonymIndex(attrRef.synonym).has_value() == false) {
      //   return false;
      // }
      // If base entity already does not have any results, terminate the process
      if (cache_.hasRows(entityType_[getSynonymIndex(attrRef.synonym).value()]) == 0) {
        return false;
      }
      resultList_.emplace_back(normalizeAttrRef(attrRef));
    }
    return true;
  }

  bool Evaluator::normalizeWithAttributes(const std::vector<With> &withList) {
    for (auto &[lhs, rhs] : withList) {
      if (std::holds_alternative<AttrRef>(lhs)) {
        const AttrRef &ref = std::get<AttrRef>(lhs);
        if (cache_.hasRows(entityType_[getSynonymIndex(ref.synonym).value()]) == 0) {
          return false;
        }
        normalizeAttrRef(ref);
      }
      if (std::holds_alternative<AttrRef>(rhs)) {
        const AttrRef &ref = std::get<AttrRef>(rhs);
        if (cache_.hasRows(entityType_[getSynonymIndex(ref.synonym).value()]) == 0) {
          return false;
        }
        normalizeAttrRef(ref);
      }
    }
    return true;
  }

};
