#include "Evaluator.h"
#include "Visitor.h"
#include <SPAAssert.h>

namespace query_eval {
  /**
   * =================================================================
   * Stage 4: Clause Evaluation
   * Methods for Pattern clauses
   * =================================================================
   */

  void Evaluator::sortPatterns(const std::vector<Pattern> &patterns) {
    for (const Pattern& pattern: patterns) {
      std::visit(visitor {
        [&](const BinaryPattern &p) {
          SchemaType synonymIndex = getSynonymIndex(static_cast<std::string>(p.synonym)).value();
          EntityId synonymEntity = entityType_.at(synonymIndex);
          if (cache_.hasRows(synonymEntity) == false) {
            hasResult_ = false;
            return;
          }
          if(synonymEntity == EntityId::Assign) {
            hasResult_ = processAssignPattern(p);
          } else if (synonymEntity == EntityId::While) {
            hasResult_ = processWhilePattern(p);
          } else {
            hasResult_ = false;
          }
        },
        [&](const TernaryPattern &p) {
          SchemaType synonymIndex = getSynonymIndex(static_cast<std::string>(p.synonym)).value();
          EntityId synonymEntity = entityType_.at(synonymIndex);
          if (cache_.hasRows(synonymEntity) == false) {
            hasResult_ = false;
            return;
          }
          if (synonymEntity == EntityId::If) {
            hasResult_ = processIfPattern(p);
          } else {
            hasResult_ = false;
          }
        }
      }, pattern);

      if (!hasResult_) return;

    }
  }

  bool Evaluator::processAssignPattern(const BinaryPattern &pattern) {
    SchemaType synonymIndex = getSynonymIndex(static_cast<std::string>(pattern.synonym)).value();
    SPA_ASSERT(entityType_.at(synonymIndex) == EntityId::Assign);

    // Check LHS of pattern
    bool hasResult = std::visit(visitor {
      [&](const Wildcard &) {
        return true;
      },
      [&](const Synonym &s) {
        auto leftSynOpt = getSynonymIndex(static_cast<std::string>(s));
        if (!leftSynOpt.has_value()) {
          return false;
        }
        SchemaType leftSynIndex = leftSynOpt.value();
        EntityId leftSynType = entityType_.at(leftSynIndex);

        return emplaceTable(
          cache_.select(RelationId::AssignLHS, EntityId::Assign, leftSynType)
          .reschema({synonymIndex, leftSynIndex})
        );
      },
      [&](const Var &v) {
        auto varNameOpt = cache_.getNameId(toString(v));
        if (!varNameOpt.has_value()) {
          return false;
        }
        SchemaType varNameId = varNameOpt.value();
        if (!cache_.contains(EntityId::Variable, varNameId)) {
          return false;
        }

        return emplaceTable(
          cache_.filterRight(RelationId::AssignLHS, EntityId::Assign, varNameId)
          .reschema({synonymIndex})
        );
      }
    }, pattern.left);

    if (!hasResult) return false;
    // Check RHS of Assign pattern
    hasResult = std::visit(visitor {
      [&](const Wildcard &) {
        return true;
      },
      [&](const PartialMatch &pm) {
        return emplaceTable(
          cache_.performAssignRhsPartialMatch(pm.expr)
          .reschema({synonymIndex})
        );
      },
      [&](const ExactMatch &em) {
        return emplaceTable(
          cache_.performAssignRhsExactMatch(em.expr)
          .reschema({synonymIndex})
        );
      }
    }, pattern.right);

    return hasResult;
  }

  bool Evaluator::processWhilePattern(const BinaryPattern &pattern) {
    SchemaType synonymIndex = getSynonymIndex(static_cast<std::string>(pattern.synonym)).value();

    SPA_ASSERT(entityType_.at(synonymIndex) == EntityId::While);
    SPA_ASSERT(std::holds_alternative<Wildcard>(pattern.right));

    return std::visit(visitor {
      [&](const Wildcard &) {
        return emplaceTable(
          cache_.selectLeft(RelationId::WhileConditional, EntityId::While)
          .reschema({synonymIndex})
        );
      },
      [&](const Synonym &s) {
        auto leftSynOpt = getSynonymIndex(static_cast<std::string>(s));
        if (!leftSynOpt.has_value()) {
          return false;
        }
        SchemaType leftSynIndex = leftSynOpt.value();
        EntityId leftSynType = entityType_.at(leftSynIndex);
        return emplaceTable(
          cache_.select(RelationId::WhileConditional, EntityId::While, leftSynType)
          .reschema({synonymIndex, leftSynIndex})
        );
      },
      [&](const Var &v) {
        auto varNameOpt = cache_.getNameId(toString(v));
        if (!varNameOpt.has_value()) {
          return false;
        }
        SchemaType varNameId = varNameOpt.value();
        if (!cache_.contains(EntityId:: Variable, varNameId)) {
          return false;
        }
        return emplaceTable(
          cache_.filterRight(RelationId::WhileConditional, EntityId::While, varNameId)
          .reschema({synonymIndex})
        );
      }
    }, pattern.left);
  }

  bool Evaluator::processIfPattern(const TernaryPattern &pattern) {
    SchemaType synonymIndex = getSynonymIndex(static_cast<std::string>(pattern.synonym)).value();

    SPA_ASSERT(entityType_.at(synonymIndex) == EntityId::If);
    return std::visit(visitor {
      [&](const Wildcard &) {
        return emplaceTable(
          cache_.selectLeft(RelationId::IfConditional, EntityId::If)
          .reschema({synonymIndex})
        );
      },
      [&](const Synonym &s) {
        auto leftSynOpt = getSynonymIndex(static_cast<std::string>(s));
        if (!leftSynOpt.has_value()) {
          return false;
        }
        SchemaType leftSynIndex = leftSynOpt.value();
        EntityId leftSynType = entityType_.at(leftSynIndex);
        return emplaceTable(
          cache_.select(RelationId::IfConditional, EntityId::If, leftSynType)
          .reschema({synonymIndex, leftSynIndex})
        );
      },
      [&](const Var &v) {
        auto varNameOpt = cache_.getNameId(toString(v));
        if (!varNameOpt.has_value()) {
          return false;
        }
        SchemaType varNameId = varNameOpt.value();
        if (!cache_.contains(EntityId::Variable, varNameId)) {
          return false;
        }
        return emplaceTable(
          cache_.filterRight(RelationId::IfConditional, EntityId::If, varNameId)
          .reschema({synonymIndex})
        );
      }
    }, pattern.left);
  }

};
