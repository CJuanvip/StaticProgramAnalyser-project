#pragma once

#include "Mapper.h"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <iostream>

namespace common {

  // Enum that stores the list of entities that would be encountered in PQL/SIMPLE program
  enum class EntityId: unsigned int {
    Statement = 0,
    ProgLine = 1,
    Read = 2,
    Print = 3,
    Call = 4,
    While = 5,
    If = 6,
    Assign = 7,
    Procedure = 8,
    Constant = 9,
    Variable = 10,
    Wildcard = 11
  };
  std::ostream& operator<<(std::ostream& os, const EntityId& entityId);

  // Stores mapping of entity name (as written in PQL) -> EntityId
  const std::unordered_map<std::string, EntityId> TYPE_STRING_ID = {
    {std::string("stmt"), EntityId::Statement},
    {std::string("prog_line"), EntityId::ProgLine},
    {std::string("read"), EntityId::Read},
    {std::string("print"), EntityId::Print},
    {std::string("call"), EntityId::Call},
    {std::string("while"), EntityId::While},
    {std::string("if"), EntityId::If},
    {std::string("assign"), EntityId::Assign},
    {std::string("procedure"), EntityId::Procedure},
    {std::string("constant"), EntityId::Constant},
    {std::string("variable"), EntityId::Variable},
    {std::string("_"), EntityId::Wildcard},
  };

  // Enum that stores the types of attributes that would be encountered in PQL
  enum class AttrNameId: unsigned int {
    ProcName = 0,
    VarName = 1,
    Value = 2,
    StmtNo = 3,
  };

  // Stores mapping of attribute name (as written in PQL) to AttrNameId
  const std::unordered_map<std::string, AttrNameId> ATTRIBUTE_STRING_ID = {
    {std::string("procName"), AttrNameId::ProcName},
    {std::string("varName"), AttrNameId::VarName},
    {std::string("value"), AttrNameId::Value},
    {std::string("stmt#"), AttrNameId::StmtNo},
  };

  // Stores the mapping of attribute name to variant index of Ref
  // Currently: using Ref = std::variant<std::string,Int,AttrRef>;
  const std::unordered_map<AttrNameId, size_t> ATTRIBUTE_ID_INDEX = {
    {AttrNameId::ProcName, 0},
    {AttrNameId::VarName, 0},
    {AttrNameId::Value, 1},
    {AttrNameId::StmtNo, 1}
  };

  // Stores mapping of entity to AttrNameId that are valid
  const std::unordered_map<EntityId,  std::unordered_set<AttrNameId>> TYPE_ID_ATTRIBUTE = {
    {EntityId::Statement, {AttrNameId::StmtNo}},
    {EntityId::Read, {AttrNameId::StmtNo, AttrNameId::VarName}},
    {EntityId::Print, {AttrNameId::StmtNo, AttrNameId::VarName}},
    {EntityId::Call, {AttrNameId::StmtNo, AttrNameId::ProcName}},
    {EntityId::While, {AttrNameId::StmtNo}},
    {EntityId::If, {AttrNameId::StmtNo}},
    {EntityId::Assign, {AttrNameId::StmtNo}},
    {EntityId::Procedure, {AttrNameId::ProcName}},
    {EntityId::Variable, {AttrNameId::VarName}},
    {EntityId::Constant, {AttrNameId::Value}},
    {EntityId::ProgLine, {}},
  };

  // Stores attributes of the entity that are equal to the attribute itself...
  const std::unordered_map<EntityId, std::unordered_set<AttrNameId>> TYPE_ID_ATTRIBUTE_EQUAL = {
    {EntityId::Statement, {AttrNameId::StmtNo}},
    {EntityId::Read, {AttrNameId::StmtNo}},
    {EntityId::Print, {AttrNameId::StmtNo}},
    {EntityId::Call, {AttrNameId::StmtNo}},
    {EntityId::While, {AttrNameId::StmtNo}},
    {EntityId::If, {AttrNameId::StmtNo}},
    {EntityId::Assign, {AttrNameId::StmtNo}},
    {EntityId::Procedure, {AttrNameId::ProcName}},
    {EntityId::Variable, {AttrNameId::VarName}},
    {EntityId::Constant, {AttrNameId::Value}},
    {EntityId::ProgLine, {}},
  };

  // Stores mapping of EntityId -> entity name (as written in PQL)
  const std::unordered_map<EntityId, std::string> TYPE_ID_STRING =
    mapper::inverseOnetoOneMapper<std::string, EntityId>(TYPE_STRING_ID);

  // Stores the list of entity types which are considered interchangable with statement
  const std::unordered_set<EntityId> STATEMENT_TYPES = {
    EntityId::Statement,
    EntityId::ProgLine,
    EntityId::Read,
    EntityId::Print,
    EntityId::Call,
    EntityId::While,
    EntityId::Assign,
    EntityId::If
  };

  // Stores the list of entity types which are actually strings/names
  const std::unordered_set<EntityId> NAME_TYPES = {
    EntityId::Procedure,
    EntityId::Constant,
    EntityId::Variable
  };

  // Stores mapping of AttrNameId to attribute name (as written in PQL)
  const std::unordered_map<AttrNameId, std::string> ATTRIBUTE_ID_STRING =
    mapper::inverseOnetoOneMapper<std::string, AttrNameId>(ATTRIBUTE_STRING_ID);

  const size_t NUM_ENTITIES = TYPE_STRING_ID.size();
}
