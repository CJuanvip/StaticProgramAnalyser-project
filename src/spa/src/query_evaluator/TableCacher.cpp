#include "TableCacher.h"
#include "pkb/PKBTable.h"
#include "utils/PKBTableManipulations.h"
#include "utils/ExpressionNameCollector.h"
#include <algorithm>
#include <SPAAssert.h>

namespace query_eval
{
  TableCacher::TableCacher(pkb::PKBTableManager &tableManager) {

    generateSynonymResultTables(tableManager);
    fetchTablesFromManager(tableManager);

    generateFunctionResultTables();

    /* Generates various nameId related tables for lookup */
    fetchAndGenerateNameTables(tableManager);


    /* Populate tables to be used for pattern clauses with AST */
    assignRhsVarTable_ = tableManager.getAssignRhs();
    assignRhsAstTable_ = tableManager.getAssignRhsAst();
  }

  void TableCacher::generateSynonymResultTables(pkb::PKBTableManager &tableManager) {
    // entityTables_.reserve(NUM_ENTITIES);

    /* Generate table for each synonym */
    auto &statementsTable = tableManager.getStatementsTable();
    // Generate table for each declaration type
    // stmt, read, print, call, while, if, assign
    entityTables_[EntityId::Statement] = statementsTable.selectLeft();
    entityTables_[EntityId::ProgLine] = statementsTable.selectLeft();

    for (auto statementType: STATEMENT_TYPES) {
      if (statementType == EntityId::Statement) continue;
      if (statementType == EntityId::ProgLine) continue;
      entityTables_[statementType] = statementsTable.filterRight(statementType);
    }

    // variable, constant, procedure
    auto &nameTypesTable = tableManager.getNameTypesTable();
    for (auto namesType: NAME_TYPES) {
      entityTables_[namesType] = nameTypesTable.filterRight(namesType);
    }

    for (const auto&[entity, entityTable]: entityTables_) {
      entityResultTables_[entity] = entityTable;
    }
  }

  void TableCacher::fetchTablesFromManager(pkb::PKBTableManager &tableManager) {
    /* Generate virtual table for functions */
    pkbRelationTables_[RelationId::Follows] = tableManager.getFollow();
    pkbRelationTables_[RelationId::FollowsT] = tableManager.getFollowClosure();
    pkbRelationTables_[RelationId::Parent] = tableManager.getParent();
    pkbRelationTables_[RelationId::ParentT] = tableManager.getParentClosure();
    pkbRelationTables_[RelationId::Modifies] = tableManager.getModifyEntity();
    pkbRelationTables_[RelationId::Uses] = tableManager.getUsesEntityVariableTable();
    pkbRelationTables_[RelationId::Calls] = tableManager.getCall();
    pkbRelationTables_[RelationId::CallsT] = tableManager.getCallClosure();
    pkbRelationTables_[RelationId::Next] = tableManager.getNext();
    pkbRelationTables_[RelationId::NextBip] = tableManager.getInterproceduralNext();
    pkbRelationTables_[RelationId::AssignLHS] = tableManager.getAssignLhs();
    pkbRelationTables_[RelationId::IfConditional] = filterPKBLeftRight(tableManager.getStatementVariables(), entityTables_[EntityId::If], entityTables_[EntityId::Variable]);
    pkbRelationTables_[RelationId::WhileConditional] = filterPKBLeftRight(tableManager.getStatementVariables(), entityTables_[EntityId::While], entityTables_[EntityId::Variable]);
    pkbRelationTables_[RelationId::ReadVar] = filterPKBLeftRight(tableManager.getStatementVariables(), entityTables_[EntityId::Read], entityTables_[EntityId::Variable]);
    pkbRelationTables_[RelationId::PrintVar] = filterPKBLeftRight(tableManager.getStatementVariables(), entityTables_[EntityId::Print], entityTables_[EntityId::Variable]);
    pkbRelationTables_[RelationId::CallProc] = filterPKBLeftRight(tableManager.getStatementVariables(), entityTables_[EntityId::Call], entityTables_[EntityId::Procedure]);

    // Realtime evaluated tables
    pkbRealtimeRelationTables_[RelationId::NextT] = &tableManager.getNextClosure();
    pkbRealtimeRelationTables_[RelationId::Affects] = &tableManager.getAffect();
    pkbRealtimeRelationTables_[RelationId::AffectsT] = &tableManager.getAffectClosure();
    pkbRealtimeRelationTables_[RelationId::NextBipT] = &tableManager.getInterproceduralNextClosure();
    pkbRealtimeRelationTables_[RelationId::AffectsBip] = &tableManager.getInterproceduralAffect();
    pkbRealtimeRelationTables_[RelationId::AffectsBipT] = &tableManager.getInterproceduralAffectClosure();
  }

  void TableCacher::resetRealtimeRelationTables() {
    selectSameCache_.clear();
    selectLeftCache_.clear();
    selectRightCache_.clear();
    selectCache_.clear();
  }

  void TableCacher::generateFunctionResultTables() {
    // Initialize all result tables as empty
    for (auto &[relationId, pkbTable]:pkbRelationTables_) {
      for (auto &[lhsName, leftEntity]: TYPE_STRING_ID) {
        for (auto &[rhsName, rightEntity]: TYPE_STRING_ID) {
          relationTableMatrix_[relationId][leftEntity][rightEntity]
           = pkb::PKBTable<CellType, CellType>();
        }
      }

      for (auto &[name, entity]: TYPE_STRING_ID) {
        relationTableFilterLeftType_[relationId][entity]
          = pkb::PKBTable<CellType, CellType>();
        relationTableFilterRightType_[relationId][entity]
          = pkb::PKBTable<CellType, CellType>();
      }
    }

    for (const auto&[relationId, relationPKB]: pkbRelationTables_) {
      for (const auto& leftEntity: RELATION_LHS.at(relationId)) {
        for (const auto& rightEntity: RELATION_RHS.at(relationId)) {
          relationTableMatrix_[relationId][leftEntity][rightEntity]
            = filterPKBLeftRight(relationPKB, entityTables_[leftEntity], entityTables_[rightEntity]);
        }
      }

      // Generate for relation(entity, _)
      std::unordered_set<CellType> rightEntityUnion;
      for (const auto& wildcardEntity: RELATION_RHS_WILDCARD.at(relationId)) {
        rightEntityUnion.insert(entityTables_[wildcardEntity].begin(), entityTables_[wildcardEntity].end());
      }
      for (const auto& leftEntity: RELATION_LHS.at(relationId)) {
        relationTableMatrix_[relationId][leftEntity][EntityId::Wildcard]
          = filterPKBLeftRight(relationPKB, entityTables_[leftEntity], rightEntityUnion);
      }

      // Generate for relation(_, entity)

      std::unordered_set<CellType> leftEntityUnion;
      for (const auto& wildcardEntity: RELATION_LHS_WILDCARD.at(relationId)) {
        leftEntityUnion.insert(entityTables_[wildcardEntity].begin(), entityTables_[wildcardEntity].end());
      }
      for (const auto& rightEntity: RELATION_RHS.at(relationId)) {
        relationTableMatrix_[relationId][EntityId::Wildcard][rightEntity]
          = filterPKBLeftRight(relationPKB, leftEntityUnion, entityTables_[rightEntity]);
      }

      // Generate for relation(_, _)
      relationTableMatrix_[relationId][EntityId::Wildcard][EntityId::Wildcard]
       = filterPKBLeftRight(relationPKB, leftEntityUnion, rightEntityUnion);

      // Generate for relation(leftType, *)
      for (const auto& leftEntity: RELATION_LHS.at(relationId)) {
        relationTableFilterLeftType_[relationId][leftEntity]
          = filterPKBLeft(relationPKB, entityTables_[leftEntity]);
      }
      relationTableFilterLeftType_[relationId][EntityId::Wildcard]
          = filterPKBLeft(relationPKB, leftEntityUnion);

      // Generate for relation(*, rightType)
      for (const auto& rightEntity: RELATION_RHS.at(relationId)) {
        relationTableFilterRightType_[relationId][rightEntity]
          = filterPKBRight(relationPKB, entityTables_[rightEntity]);
      }
      relationTableFilterRightType_[relationId][EntityId::Wildcard]
          = filterPKBRight(relationPKB, rightEntityUnion);
    }

    for (auto &[relationId, pkbTable]:pkbRelationTables_) {
      for (auto &[lhsName, leftEntity]: TYPE_STRING_ID) {
        for (auto &[rhsName, rightEntity]: TYPE_STRING_ID) {
          // relationTableMatrix_[relationId][leftEntity][rightEntity]
          relationTableMatrixSelect_[relationId][leftEntity][rightEntity]
            = relationTableMatrix_[relationId][leftEntity][rightEntity].select();

          relationTableMatrixLeft_[relationId][leftEntity][rightEntity]
            = relationTableMatrix_[relationId][leftEntity][rightEntity].selectLeft();

          relationTableMatrixRight_[relationId][leftEntity][rightEntity]
            = relationTableMatrix_[relationId][leftEntity][rightEntity].selectRight();
        }
      }
    }

    // Generate selectSame tables
    for (auto &[relation, pkbTable]:pkbRelationTables_) {
      for (auto &[entityName, entity]: TYPE_STRING_ID) {
        if (entity == EntityId::Wildcard) continue;
        const auto &valid = entityTables_.at(entity);
        std::unordered_set<CellType> result;
        for (const auto &x: valid) {
          const auto &rightSide = pkbTable.filterLeft(x);
          if (rightSide.count(x) == 1) {
            result.emplace(x);
          }
        }
        selectSameTables_[relation][entity] = result;
      }
    }

  }

  void TableCacher::fetchAndGenerateNameTables(pkb::PKBTableManager &tableManager) {
    namesTable_ = tableManager.getNamesTable();
    SPA_ASSERT(namesTable_.selectLeft().size() == namesTable_.count());
    CellType maxNameId = 0;
    for (auto &nameId : namesTable_.selectLeft()) {
      maxNameId = std::max(maxNameId, nameId);
    }
    nameIdtoStringTable_.resize(maxNameId+1);
    for (auto &row : namesTable_.select()) {
      nameIdtoStringTable_[row.getLeft()] = row.getRight();
    }
  }

  std::optional<CellType> TableCacher::getNameId(const std::string &nameString) const {
    const auto &result = namesTable_.filterRight(nameString);
    if (result.size() == 0) return {};
    SPA_ASSERT(result.size() == 1);
    return *result.begin();
  }

  const std::string& TableCacher::getName(const CellType &nameId) const {
    return nameIdtoStringTable_[nameId];
  }

  ResultTable TableCacher::performAssignRhsExactMatch(const Expr& expr) const {
    std::string exprString = toString(expr);
    return assignRhsAstTable_.filterRight(exprString);
  }

  ResultTable TableCacher::performAssignRhsPartialMatch(const Expr& expr) const {

    if (std::holds_alternative<Var>(expr)) {
      auto varNameOpt = getNameId(toString(std::get<Var>(expr)));
      if (!varNameOpt.has_value()) {
        return empty_;
      }
      SchemaType varNameId = varNameOpt.value();
      return assignRhsVarTable_.filterRight(varNameId);
    } else if (std::holds_alternative<Const> (expr)) {
      auto constNameOpt = getNameId(toString(std::get<Const>(expr)));
      if (!constNameOpt.has_value()) {
        return empty_;
      }
      SchemaType constNameId = constNameOpt.value();
      return assignRhsVarTable_.filterRight(constNameId);
    } else {
      ExpressionNameCollector collector;
      std::visit(collector, expr);
      std::vector<CellType> nameIds;
      for (const auto&[name, freq]: collector.getCollectedNames()) {
        auto nameOpt = getNameId(name);
        if (!nameOpt.has_value()) {
          return empty_;
        } else {
          nameIds.emplace_back(nameOpt.value());
        }
      }
      SPA_ASSERT(nameIds.size() > 0);
      std::sort(nameIds.begin(), nameIds.end(), [&](const auto& a, const auto& b) {
        return assignRhsVarTable_.countWithRight(a) < assignRhsVarTable_.countWithRight(b);
      });
      if (assignRhsVarTable_.countWithRight(nameIds[0]) == 0) {
        return empty_;
      }
      std::vector<CellType> shortlist;
      for (const auto& assignId: assignRhsVarTable_.filterRight(nameIds[0])) {
        shortlist.emplace_back(assignId);
      }
      for (size_t i = 1; i < nameIds.size() && shortlist.size() > nameIds.size() - i; ++i) {
        auto it = std::remove_if(shortlist.begin(), shortlist.end(), [&](auto a) {
          return !assignRhsVarTable_.contains(a, nameIds[i]);
        });
        shortlist.resize(it - shortlist.begin());
        if (shortlist.size() == 0) {
          return empty_;
        }
      }
      std::string exprString = toString(expr);
      auto it = std::remove_if(shortlist.begin(), shortlist.end(), [&](auto assignId) {
        const std::string ast = *(assignRhsAstTable_.filterLeft(assignId).begin());
        if (ast.size() < exprString.size()) return true;
        std::size_t found = ast.find(exprString);
        return found == std::string::npos; // If not found --> remove
      });
      shortlist.resize(it - shortlist.begin());
      return shortlist;
    }
  }


  // Relation functions
  // Returns true if Relation(_, _) is true
  bool TableCacher::hasWildWild(RelationId relation) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      auto table = pkbRealtimeRelationTables_.at(relation);
      return table->count() > 0;
    }
    return relationTableMatrix_.at(relation).at(EntityId::Wildcard).at(EntityId::Wildcard).count() > 0;
  }
  // Returns true if the LHS of the relation contains left with right side being Wildcard
  bool TableCacher::hasLeftWild(RelationId relation, CellType left) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      return pkbRealtimeRelationTables_.at(relation)->countWithLeft(left) > 0;
    }
    return relationTableFilterRightType_.at(relation).at(EntityId::Wildcard).countWithLeft(left) > 0;
  }
  // Returns true if the RHS of the relation contains right with left hand side being Wildcard
  bool TableCacher::hasWildRight(RelationId relation, CellType right) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      return pkbRealtimeRelationTables_.at(relation)->countWithRight(right) > 0;
    }
    return relationTableFilterLeftType_.at(relation).at(EntityId::Wildcard).countWithRight(right) > 0;
  }
  // Returns true if the relation contains the given pair
  bool TableCacher::contains(RelationId relation, CellType left, CellType right) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      return pkbRealtimeRelationTables_.at(relation)->contains(left, right);
    }
    return pkbRelationTables_.at(relation).contains(left, right);
  }

  // Obtains only the left of the table
  ResultTable TableCacher::selectLeft(RelationId relation, EntityId leftEntity)  {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (selectLeftCache_[relation].count(leftEntity) != 0) {
        return selectLeftCache_[relation].at(leftEntity);
      }
      if (leftEntity == EntityId::Wildcard) {
        return selectLeftCache_[relation][leftEntity]
          = pkbRealtimeRelationTables_.at(relation)->selectLeft();
      }
      return selectLeftCache_[relation][leftEntity]
        = filterPKBLeft(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(leftEntity)).selectLeft();
    }
    return relationTableMatrixLeft_.at(relation).at(leftEntity).at(EntityId::Wildcard);
  }
  // Obtains only the right of the table
  ResultTable TableCacher::selectRight(RelationId relation, EntityId rightEntity) {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (selectRightCache_[relation].count(rightEntity) != 0) {
        return selectRightCache_[relation].at(rightEntity);
      }
      if (rightEntity == EntityId::Wildcard) {
        return selectRightCache_[relation][rightEntity]
          = pkbRealtimeRelationTables_.at(relation)->selectRight();
      }
      return selectRightCache_[relation][rightEntity]
          = filterPKBRight(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(rightEntity)).selectRight();
    }
    return relationTableMatrixRight_.at(relation).at(EntityId::Wildcard).at(rightEntity);
  }
  // Obtains only the values where both are the same
  ResultTable TableCacher::selectSame(RelationId relation, EntityId entity)  {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (selectSameCache_[relation].count(entity) != 0) {
        return selectSameCache_[relation].at(entity);
      }
      if (entity == EntityId::Wildcard) {
        const auto &valid = pkbRealtimeRelationTables_.at(relation)->selectLeft();
        std::unordered_set<CellType> result;
        for (const auto &x: valid) {
          if (pkbRealtimeRelationTables_.at(relation)->contains(x, x)) {
            result.emplace(x);
          }
        }
        return selectSameCache_[relation][entity] = result;
      }
      const auto &valid = entityTables_.at(entity);
      std::unordered_set<CellType> result;
      for (const auto &x: valid) {
        const auto &rightSide = pkbRealtimeRelationTables_.at(relation)->filterLeft(x);
        if (rightSide.count(x) == 1) {
          result.emplace(x);
        }
      }
      return selectSameCache_[relation][entity] = result;
    }
    return selectSameTables_.at(relation).at(entity);
  }

  // Obtains the left of the table, filtered by left type, right value
  ResultTable TableCacher::filterRight(RelationId relation, EntityId leftEntity, CellType right) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (leftEntity == EntityId::Wildcard) {
        return filterPKBRight(*pkbRealtimeRelationTables_.at(relation), {right}).selectLeft();
      }
      return filterPKBLeftRight(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(leftEntity), {right}).selectLeft();
    }
    return relationTableFilterLeftType_.at(relation).at(leftEntity).filterRight(right);
  }
  // Obtains the right of the table, filtered by left value, right type
  ResultTable TableCacher::filterLeft(RelationId relation, CellType left, EntityId rightEntity) const {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (rightEntity == EntityId::Wildcard) {
        return filterPKBLeft(*pkbRealtimeRelationTables_.at(relation), {left}).selectRight();
      }
      return filterPKBLeftRight(*pkbRealtimeRelationTables_.at(relation), {left}, entityTables_.at(rightEntity)).selectRight();
    }
    return relationTableFilterRightType_.at(relation).at(rightEntity).filterLeft(left);
  }
  // Obtains the entire table
  ResultTable TableCacher::select(RelationId relation, EntityId leftEntity, EntityId rightEntity) {
    if (RELATION_REALTIME.count(relation) == 1) {
      if (selectCache_[relation][leftEntity].count(rightEntity) != 0) {
        return selectCache_[relation].at(leftEntity).at(rightEntity);
      }
      if (leftEntity == EntityId::Wildcard && rightEntity == EntityId::Wildcard) {
        return selectCache_[relation][leftEntity][rightEntity]
          = pkbRealtimeRelationTables_.at(relation)->select();
      } else if (leftEntity == EntityId::Wildcard) {
        return selectCache_[relation][leftEntity][rightEntity]
          = filterPKBRight(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(rightEntity)).select();
      } else if (rightEntity == EntityId::Wildcard) {
        return selectCache_[relation][leftEntity][rightEntity]
          = filterPKBLeft(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(leftEntity)).select();
      } else {
        return selectCache_[relation][leftEntity][rightEntity]
          = filterPKBLeftRight(*pkbRealtimeRelationTables_.at(relation), entityTables_.at(leftEntity), entityTables_.at(rightEntity)).select();
      }
    }
    return relationTableMatrixSelect_.at(relation).at(leftEntity).at(rightEntity);
  }

  // Entity functions
  // Returns true if the entity exists
  bool TableCacher::hasRows(EntityId entity) const {
    return entityTables_.at(entity).size() > 0;
  }
  // Returns true if there the entity attribute exists
  bool TableCacher::hasRows(EntityId entity, std::optional<AttrNameId> attribute) const {
    if (attribute.has_value()) {
      if (TYPE_ID_ATTRIBUTE.at(entity).count(attribute.value()) == 0) return false;
      if (TYPE_ID_ATTRIBUTE_EQUAL.at(entity).count(attribute.value()) == 0) {
        if (entity == EntityId::Read && attribute.value() == AttrNameId::VarName) {
          return pkbRelationTables_.at(RelationId::ReadVar).count() > 0;
        } else if (entity == EntityId::Print && attribute.value() == AttrNameId::VarName) {
          return pkbRelationTables_.at(RelationId::PrintVar).count() > 0;
        } else {
          SPA_ASSERT(entity == EntityId::Call && attribute.value() == AttrNameId::ProcName);
          return pkbRelationTables_.at(RelationId::CallProc).count() > 0;
        }
      }
    }
    return hasRows(entity);
  }

  // Returns true if the given value is of this entity
  bool TableCacher::contains(EntityId entity, CellType value) const {
    return entityTables_.at(entity).count(value) > 0;
  }
  // Returns true if there exists an entity attribute of the given value
  bool TableCacher::contains(EntityId entity, std::optional<AttrNameId> attribute, CellType value) const {
    if (attribute.has_value()) {
      if (TYPE_ID_ATTRIBUTE.at(entity).count(attribute.value()) == 0) return false;
      if (TYPE_ID_ATTRIBUTE_EQUAL.at(entity).count(attribute.value()) == 0) {
        if (entity == EntityId::Read && attribute.value() == AttrNameId::VarName) {
          return pkbRelationTables_.at(RelationId::ReadVar).countWithRight(value) > 0;
        } else if (entity == EntityId::Print && attribute.value() == AttrNameId::VarName) {
          return pkbRelationTables_.at(RelationId::PrintVar).countWithRight(value) > 0;
        } else {
          SPA_ASSERT(entity == EntityId::Call && attribute.value() == AttrNameId::ProcName);
          return pkbRelationTables_.at(RelationId::CallProc).countWithRight(value) > 0;
        }
      }
    }
    return contains(entity, value);
  }
  // Returns the entire list of the entity
  const ResultTable& TableCacher::select(EntityId entity) const {
    return entityResultTables_.at(entity);
  }
  // Returns the entire list of the entity attribute
  const ResultTable& TableCacher::select(EntityId entity, std::optional<AttrNameId> attribute) const {
    if (attribute.has_value()) {
      SPA_ASSERT(TYPE_ID_ATTRIBUTE.at(entity).count(attribute.value()) != 0);
      if (TYPE_ID_ATTRIBUTE_EQUAL.at(entity).count(attribute.value()) == 0) {
        if (entity == EntityId::Read && attribute.value() == AttrNameId::VarName) {
          return relationTableMatrixRight_.at(RelationId::ReadVar).at(EntityId::Read).at(EntityId::Variable);
        } else if (entity == EntityId::Print && attribute.value() == AttrNameId::VarName) {
          return relationTableMatrixRight_.at(RelationId::PrintVar).at(EntityId::Print).at(EntityId::Variable);
        } else {
          SPA_ASSERT(entity == EntityId::Call && attribute.value() == AttrNameId::ProcName);
          return relationTableMatrixRight_.at(RelationId::CallProc).at(EntityId::Call).at(EntityId::Procedure);
        }
      }
    }
    return entityResultTables_.at(entity);
  }



}
