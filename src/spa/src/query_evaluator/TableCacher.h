#pragma once

#include "common/EntityId.h"
#include "common/RelationId.h"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <list>
#include "ResultTable.h"
#include "pkb/PKBTableManager.h"
#include <optional>
#include "parser/AST.h"


namespace query_eval
{
  using namespace common;
  class TableCacher {
  public:
    TableCacher(pkb::PKBTableManager &tableManager);

    std::optional<CellType> getNameId(const std::string &nameString) const;
    const std::string& getName(const CellType &nameId) const;

    // Relation functions
    // Returns true if Relation(_, _) is true
    bool hasWildWild(RelationId relation) const;
    // Returns true if the LHS of the relation contains left with right side being Wildcard
    bool hasLeftWild(RelationId relation, CellType left) const;
    // Returns true if the RHS of the relation contains right with left hand side being Wildcard
    bool hasWildRight(RelationId relation, CellType right) const;
    // Returns true if the relation contains the given pair
    bool contains(RelationId relation, CellType left, CellType right) const;

    // Obtains only the left of the table
    ResultTable selectLeft(RelationId relation, EntityId leftEntity=EntityId::Wildcard);
    // Obtains only the right of the table
    ResultTable selectRight(RelationId relation, EntityId rightEntity=EntityId::Wildcard);
    // Obtains only the values where both are the same
    ResultTable selectSame(RelationId relation, EntityId entity=EntityId::Wildcard);

    // Obtains the left of the table, filtered by left tyoe, right value
    ResultTable filterRight(RelationId relation, EntityId leftEntity, CellType right) const;
    // Obtains the right of the table, filtered by left value, right type
    ResultTable filterLeft(RelationId relation, CellType left, EntityId rightEntity) const;

    // Obtains the entire table
    ResultTable select(RelationId relation, EntityId leftEntity=EntityId::Wildcard, EntityId rightEntity=EntityId::Wildcard);

    // Entity functions
    // Returns true if the entity exists
    bool hasRows(EntityId entity) const;
    // Returns true if there the entity attribute exists
    bool hasRows(EntityId entity, std::optional<AttrNameId> attribute) const;
    // Returns true if the given value is of this entity
    bool contains(EntityId entity, CellType value) const;
    // Returns true if there exists an entity attribute of the given value
    bool contains(EntityId entity, std::optional<AttrNameId> attribute, CellType value) const;
    // Returns the entire list of the entity
    const ResultTable& select(EntityId entity) const;
    // Returns the entire list of the entity attribute
    const ResultTable& select(EntityId entity, std::optional<AttrNameId> attribute) const;


    // Support Assign Pattern clauses
    ResultTable performAssignRhsExactMatch(const Expr& e) const;
    ResultTable performAssignRhsPartialMatch(const Expr& e) const;

    void resetRealtimeRelationTables();
  private:
    std::unordered_map<EntityId, std::unordered_set<CellType>> entityTables_;
    std::unordered_map<EntityId, ResultTable> entityResultTables_;
    std::unordered_map<RelationId, pkb::PKBTable<CellType, CellType>> pkbRelationTables_;
    std::unordered_map<RelationId, std::unordered_map<EntityId, std::unordered_map<EntityId, pkb::PKBTable<CellType, CellType>>>> relationTableMatrix_;

    std::unordered_map<RelationId, std::unordered_map<EntityId, std::unordered_map<EntityId, ResultTable>>> relationTableMatrixSelect_;
    std::unordered_map<RelationId, std::unordered_map<EntityId, std::unordered_map<EntityId, ResultTable>>> relationTableMatrixLeft_;
    std::unordered_map<RelationId, std::unordered_map<EntityId, std::unordered_map<EntityId, ResultTable>>> relationTableMatrixRight_;

    std::unordered_map<RelationId, std::unordered_map<EntityId, pkb::PKBTable<CellType, CellType>>> relationTableFilterLeftType_; // stores left of a certain type, left any
    std::unordered_map<RelationId, std::unordered_map<EntityId, pkb::PKBTable<CellType, CellType>>> relationTableFilterRightType_; // stores right of a certain type, right any

    std::unordered_map<RelationId, pkb::RealtimePKBTable<CellType, CellType>*> pkbRealtimeRelationTables_;

    std::unordered_map<RelationId, std::unordered_map<EntityId, ResultTable>> selectSameTables_;

    // Cache for realtime tables
    std::unordered_map<RelationId, std::unordered_map<EntityId, ResultTable>> selectSameCache_, selectLeftCache_, selectRightCache_;
    std::unordered_map<RelationId, std::unordered_map<EntityId, std::unordered_map<EntityId, ResultTable>>> selectCache_;

    ResultTable empty_;

    pkb::PKBTable<CellType, std::string> namesTable_;

    pkb::PKBTable<CellType, CellType> assignRhsVarTable_;
    pkb::PKBTable<CellType, std::string> assignRhsAstTable_;

    void generateSynonymResultTables(pkb::PKBTableManager &tableManager);

    void fetchTablesFromManager(pkb::PKBTableManager &tableManager);

    void generateFunctionResultTables();
    void fetchAndGenerateNameTables(pkb::PKBTableManager &tableManager);
    std::vector<std::string> nameIdtoStringTable_;

  };
}
