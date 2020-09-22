#include "Evaluator.h"
#include "Visitor.h"
#include <unordered_map>

namespace query_eval {

  /**
   * =================================================================
   * Stage 1: PQL validation
   * =================================================================
   */

  bool Evaluator::hasSemanticError(const Query& query) const {
    std::unordered_map<Synonym, EntityId> synonymSet;
    // Check for repeated synonym names
    for (const auto &declaration: query.declarations) {
      if (synonymSet.count(declaration.synonym) != 0) {
        // Two declaration with same synonym name
        return true;
      }
      synonymSet[declaration.synonym] = declaration.type;
    }

    // Check if result is declared
    for (const auto &result: query.result) {
      if (synonymSet.count(result.synonym) != 1) {
        return true;
      }
      if (result.attribute.has_value()) {
        if (TYPE_ID_ATTRIBUTE.at(synonymSet[result.synonym]).count(result.attribute.value()) == 0) {
          // Not a valid attribute of the synonym type
          return true;
        }
      }
    }

    // Check if suchThats have valid synonyms
    for (const auto&suchThat: query.suchThats) {
      // Obtain the type of the relation
      const RelationId &relation = std::visit(visitor {
        [&](const auto& clause) { return clause.type; }
      }, suchThat);

      // Checks that both left and right are valid synonyms
      std::optional<Synonym> leftSynonym = std::visit(visitor {
        [&](const StmtEntEntRelation& clause) {
          return std::visit(visitor{
            [&](const auto& ref) {
              if (std::holds_alternative<Synonym>(ref)) {
                return (std::optional<Synonym>) std::get<Synonym>(ref);
              } else {
                return std::optional<Synonym>{};
              }
            }
          }, clause.left);
        },
        [&](const auto& clause) {
          if (std::holds_alternative<Synonym>(clause.left)) {
            return (std::optional<Synonym>) std::get<Synonym>(clause.left);
          } else {
            return std::optional<Synonym>{};
          }
        }
      }, suchThat);

      // Check if synonym is declared, of correct LHS type
      if (leftSynonym.has_value()) {
        if (synonymSet.count(leftSynonym.value()) != 1) {
          return true;
        }
        // EntityType of LHS not allowed for this relation
        if (RELATION_LHS.at(relation).count(synonymSet.at(leftSynonym.value())) != 1) {
          return true;
        }
      }

      std::optional<Synonym> rightSynonym = std::visit(visitor{
        [&](const auto& clause) {
          if (std::holds_alternative<Synonym>(clause.right)) {
            return (std::optional<Synonym>) std::get<Synonym>(clause.right);
          } else {
            return std::optional<Synonym>{};
          }
        }
      }, suchThat);
      // Check if synonym is declared
      if (rightSynonym.has_value()) {
        if (synonymSet.count(rightSynonym.value()) != 1) {
          return true;
        }
        // EntityType of RHS not allowed for this relation
        if (RELATION_RHS.at(relation).count(synonymSet.at(rightSynonym.value())) != 1) {
          return true;
        }
      }

      // Check LHS wildcard is not allowed
      if (RELATION_LHS_WILDCARD.at(relation).size() == 0) {
        bool hasWildcard = std::visit(visitor{
          [&](const StmtEntEntRelation& clause) {
            return std::visit(visitor{
              [&](const auto& ref) {
                return std::holds_alternative<Wildcard>(ref);
              }
            }, clause.left);
          },
          [&](const auto& clause) {
            return std::holds_alternative<Wildcard>(clause.left);
          }
        }, suchThat);
        // Wildcard is present in LHS, restrict
        if (hasWildcard) return true;
      }

      // Check RHS wildcard is not allowed
      if (RELATION_RHS_WILDCARD.at(relation).size() == 0) {
        bool hasWildcard = std::visit(visitor{
          [&](const auto& clause) {
            return std::holds_alternative<Wildcard>(clause.right);
          }
        }, suchThat);
        // Wildcard is present in RHS, restrict
        if (hasWildcard) return true;
      }
    }

    for (const auto &pattern:query.patterns) {
      bool fail = std::visit(visitor {
        [&](const BinaryPattern& p) {
          // Synonym is not declared
          if (synonymSet.count(p.synonym) != 1) {
            return true;
          }
          // Just for iteration 1, only allow assign
          if (synonymSet.at(p.synonym) == EntityId::Assign) {
            // If the LHS is a synonym, it should be declared
            if (std::holds_alternative<Synonym>(p.left)) {
              Synonym leftSynonym = std::get<Synonym>(p.left);
              if (synonymSet.count(leftSynonym) == 0) {
                return true;
              }
              // LHS of assign must be a variable
              if (synonymSet.at(leftSynonym) != EntityId::Variable) {
                return true;
              }
            }
          } else if (synonymSet.at(p.synonym) == EntityId::While) {
            // If the LHS is a synonym, it should be declared
            if (std::holds_alternative<Synonym>(p.left)) {
              Synonym leftSynonym = std::get<Synonym>(p.left);
              if (synonymSet.count(leftSynonym) == 0) {
                return true;
              }
              // LHS of if must be a variable since 'entRef' does not allow for numeric constants
              if (synonymSet.at(leftSynonym) != EntityId::Variable) {
                return true;
              }
            }
            // RHS of while pattern must be Wildcard
            if (!std::holds_alternative<Wildcard>(p.right)) {
              return true;
            }
          }  else {
            return true;
          }
          return false;
        },
        [&] (const TernaryPattern& p) {
          if (synonymSet.at(p.synonym) == EntityId::If) {
            // If the LHS is a synonym, it should be declared
            if (std::holds_alternative<Synonym>(p.left)) {
              Synonym leftSynonym = std::get<Synonym>(p.left);
              if (synonymSet.count(leftSynonym) == 0) {
                return true;
              }
              // LHS of if must be a variable since 'entRef' does not allow for numeric constants
              if (synonymSet.at(leftSynonym) != EntityId::Variable) {
                return true;
              }
            }
            // Parser already ensures the remaining 2 arguments are wildcard
          }  else {
            return true;
          }
          return false;
        }
      }, pattern);
      if (fail) return true;
    }


    for (const auto &[lhs, rhs]:query.withs) {
      size_t leftType = lhs.index(), rightType = rhs.index();
      if (std::holds_alternative<AttrRef>(lhs)) {
        const AttrRef &ref = std::get<AttrRef>(lhs);
        const Synonym &synonym = ref.synonym;
        if (synonymSet.count(synonym) == 0) {
          return true;
        }
        EntityId synonymEntity = synonymSet.at(synonym);
        if (synonymEntity == EntityId::ProgLine) {
          leftType = 1;
        } else {
          // Only ProgLine can be used directly in with clause
          if (!ref.attribute.has_value()) {
            return true;
          }
          const AttrNameId &attrType = ref.attribute.value();
          // This entity does not have this particular attribute type
          if (TYPE_ID_ATTRIBUTE.at(synonymEntity).count(attrType) == 0) {
            return true;
          }
          leftType = ATTRIBUTE_ID_INDEX.at(attrType);
        }
      }
      if (std::holds_alternative<AttrRef>(rhs)) {
        const AttrRef &ref = std::get<AttrRef>(rhs);
        const Synonym &synonym = ref.synonym;
        if (synonymSet.count(synonym) == 0) {
          return true;
        }
        EntityId synonymEntity = synonymSet.at(synonym);
        if (synonymEntity == EntityId::ProgLine) {
          rightType = 1;
        } else {
          // Only ProgLine can be used directly in with clause
          if (!ref.attribute.has_value()) {
            return true;
          }
          const AttrNameId &attrType = ref.attribute.value();
          // This entity does not have this particular attribute type
          if (TYPE_ID_ATTRIBUTE.at(synonymEntity).count(attrType) == 0) {
            return true;
          }
          rightType = ATTRIBUTE_ID_INDEX.at(attrType);
        }
      }
      // Type of both sides of the with must be 'comparable' to each other
      // eg: string = string, int = int
      if (leftType != rightType) {
        return true;
      }
    }
    return false;
  }
};
