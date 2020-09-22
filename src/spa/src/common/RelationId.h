#pragma once

#include "Mapper.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "EntityId.h"

namespace common {

  // Enum that stores the types of Relations that would be encountered in PQL
  enum class RelationId: unsigned int {
    Follows = 0,
    FollowsT = 1,
    Parent = 2,
    ParentT = 3,
    Uses = 4,
    Modifies = 5,
    Calls = 6,
    CallsT = 7,
    Next = 8,
    NextT = 9,
    Affects = 10,
    AffectsT = 11,
    NextBip = 12,
    NextBipT = 13,
    AffectsBip = 14,
    AffectsBipT = 15,
    AssignLHS = 16,
    IfConditional = 17,
    WhileConditional = 18,
    ReadVar = 19,
    PrintVar = 20,
    CallProc = 21
  };

  // Enum that stores the different categories of Relations that would be encountered in PQL
  enum class RelationTypeId: unsigned int {
    Stmt_Stmt = 0,    // Both LHS and RHS are statementIds/proc_line
    StmtProc_Var = 1, // LHS can be either statementIds/proc_line/procedure, RHS must be a variable
    Proc_Proc = 2,    // Both LHS and RHS must be procedure
    Virtual = 3,      // Not an actual relation, just defined to help with answering pattern queries
  };

  // Stores mapping of Relation name (as written in PQL) to RelationId
  const std::unordered_map<std::string, RelationId> RELATION_STRING_ID = {
    {std::string("Follows"), RelationId::Follows},
    {std::string("Follows*"), RelationId::FollowsT},
    {std::string("Parent"), RelationId::Parent},
    {std::string("Parent*"), RelationId::ParentT},
    {std::string("Uses"), RelationId::Uses},
    {std::string("Modifies"), RelationId::Modifies},
    {std::string("Calls"), RelationId::Calls},
    {std::string("Calls*"), RelationId::CallsT},
    {std::string("Next"), RelationId::Next},
    {std::string("Next*"), RelationId::NextT},
    {std::string("Affects"), RelationId::Affects},
    {std::string("Affects*"), RelationId::AffectsT},
    {std::string("NextBip"), RelationId::NextBip},
    {std::string("NextBip*"), RelationId::NextBipT},
    {std::string("AffectsBip"), RelationId::AffectsBip},
    {std::string("AffectsBip*"), RelationId::AffectsBipT},
  };

  // Stores mapping of RelationId to RelationTypeId
  const std::unordered_map<RelationId, RelationTypeId> RELATION_TYPE = {
    {RelationId::Follows, RelationTypeId::Stmt_Stmt},
    {RelationId::FollowsT, RelationTypeId::Stmt_Stmt},
    {RelationId::Parent, RelationTypeId::Stmt_Stmt},
    {RelationId::ParentT, RelationTypeId::Stmt_Stmt},
    {RelationId::Uses, RelationTypeId::StmtProc_Var},
    {RelationId::Modifies, RelationTypeId::StmtProc_Var},
    {RelationId::Calls, RelationTypeId::Proc_Proc},
    {RelationId::CallsT, RelationTypeId::Proc_Proc},
    {RelationId::Next, RelationTypeId::Stmt_Stmt},
    {RelationId::NextT, RelationTypeId::Stmt_Stmt},
    {RelationId::Affects, RelationTypeId::Stmt_Stmt},
    {RelationId::AffectsT, RelationTypeId::Stmt_Stmt},
    {RelationId::NextBip, RelationTypeId::Stmt_Stmt},
    {RelationId::NextBipT, RelationTypeId::Stmt_Stmt},
    {RelationId::AffectsBip, RelationTypeId::Stmt_Stmt},
    {RelationId::AffectsBipT, RelationTypeId::Stmt_Stmt},
    {RelationId::AssignLHS, RelationTypeId::Virtual},
    {RelationId::IfConditional, RelationTypeId::Virtual},
    {RelationId::WhileConditional, RelationTypeId::Virtual},
    {RelationId::ReadVar, RelationTypeId::Virtual},
    {RelationId::PrintVar, RelationTypeId::Virtual},
    {RelationId::CallProc, RelationTypeId::Virtual},
  };

  // Stores mapping of RelationId to allowed LHS types
  const std::unordered_map<RelationId, std::unordered_set<EntityId>> RELATION_LHS = {
    {RelationId::Follows, STATEMENT_TYPES},
    {RelationId::FollowsT, STATEMENT_TYPES},
    {RelationId::Parent, {EntityId::Statement, EntityId::ProgLine, EntityId::If, EntityId::While}},
    {RelationId::ParentT, {EntityId::Statement, EntityId::ProgLine, EntityId::If, EntityId::While}},
    {RelationId::Uses, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign, EntityId::Print, EntityId::If, EntityId::While, EntityId::Procedure, EntityId::Call}},
    {RelationId::Modifies, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign, EntityId::Read, EntityId::If, EntityId::While, EntityId::Procedure, EntityId::Call}},
    {RelationId::Calls, {EntityId::Procedure}},
    {RelationId::CallsT, {EntityId::Procedure}},
    {RelationId::Next, STATEMENT_TYPES},
    {RelationId::NextT, STATEMENT_TYPES},
    {RelationId::Affects, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AffectsT, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::NextBip, STATEMENT_TYPES},
    {RelationId::NextBipT, STATEMENT_TYPES},
    {RelationId::AffectsBip, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AffectsBipT, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AssignLHS, {EntityId::Assign}},
    {RelationId::IfConditional, {EntityId::If}},
    {RelationId::WhileConditional, {EntityId::While}},
    {RelationId::ReadVar, {EntityId::Read}},
    {RelationId::PrintVar, {EntityId::Print}},
    {RelationId::CallProc, {EntityId::Call}}
  };

  // Stores mapping of RelationId to allowed RHS types
  const std::unordered_map<RelationId, std::unordered_set<EntityId>> RELATION_RHS = {
    {RelationId::Follows, STATEMENT_TYPES},
    {RelationId::FollowsT, STATEMENT_TYPES},
    {RelationId::Parent, STATEMENT_TYPES},
    {RelationId::ParentT, STATEMENT_TYPES},
    {RelationId::Uses, {EntityId::Variable}},
    {RelationId::Modifies, {EntityId::Variable}},
    {RelationId::Calls, {EntityId::Procedure}},
    {RelationId::CallsT, {EntityId::Procedure}},
    {RelationId::Next, STATEMENT_TYPES},
    {RelationId::NextT, STATEMENT_TYPES},
    {RelationId::Affects, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AffectsT, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::NextBip, STATEMENT_TYPES},
    {RelationId::NextBipT, STATEMENT_TYPES},
    {RelationId::AffectsBip, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AffectsBipT, {EntityId::Statement, EntityId::ProgLine, EntityId::Assign}},
    {RelationId::AssignLHS, {EntityId::Variable}},
    {RelationId::IfConditional, {EntityId::Variable}},
    {RelationId::WhileConditional, {EntityId::Variable}},
    {RelationId::ReadVar, {EntityId::Variable}},
    {RelationId::PrintVar, {EntityId::Variable}},
    {RelationId::CallProc, {EntityId::Procedure}}
  };

  // Stores what the LHS wildcards are equilvalent to the union of..
  const std::unordered_map<RelationId, std::unordered_set<EntityId>> RELATION_LHS_WILDCARD = {
    {RelationId::Follows, {EntityId::Statement}},
    {RelationId::FollowsT, {EntityId::Statement}},
    {RelationId::Parent, {EntityId::Statement}},
    {RelationId::ParentT, {EntityId::Statement}},
    {RelationId::Uses, {}},       // Not allowed
    {RelationId::Modifies, {}},   // Not allowed
    {RelationId::Calls, {EntityId::Procedure}},
    {RelationId::CallsT, {EntityId::Procedure}},
    {RelationId::Next, {EntityId::Statement}},
    {RelationId::NextT, {EntityId::Statement}},
    {RelationId::Affects, {EntityId::Assign}},
    {RelationId::AffectsT, {EntityId::Assign}},
    {RelationId::NextBip, {EntityId::Statement}},
    {RelationId::NextBipT, {EntityId::Statement}},
    {RelationId::AffectsBip, {EntityId::Assign}},
    {RelationId::AffectsBipT, {EntityId::Assign}},
    {RelationId::AssignLHS, {}},  // Not allowed to have wildcard
    {RelationId::IfConditional, {}}, // Not allowed to have wildcard
    {RelationId::WhileConditional, {}}, // Not allowed to have wildcard
    {RelationId::ReadVar, {}},
    {RelationId::PrintVar, {}},
    {RelationId::CallProc, {}}
  };

  // Stores what the RHS wildcards are equilvalent to the union of..
  const std::unordered_map<RelationId, std::unordered_set<EntityId>> RELATION_RHS_WILDCARD = {
    {RelationId::Follows, {EntityId::Statement}},
    {RelationId::FollowsT, {EntityId::Statement}},
    {RelationId::Parent, {EntityId::Statement}},
    {RelationId::ParentT, {EntityId::Statement}},
    {RelationId::Uses, {EntityId::Variable}},
    {RelationId::Modifies, {EntityId::Variable}},
    {RelationId::Calls, {EntityId::Procedure}},
    {RelationId::CallsT, {EntityId::Procedure}},
    {RelationId::Next, {EntityId::Statement}},
    {RelationId::NextT, {EntityId::Statement}},
    {RelationId::Affects, {EntityId::Assign}},
    {RelationId::AffectsT, {EntityId::Assign}},
    {RelationId::NextBip, {EntityId::Statement}},
    {RelationId::NextBipT, {EntityId::Statement}},
    {RelationId::AffectsBip, {EntityId::Assign}},
    {RelationId::AffectsBipT, {EntityId::Assign}},
    {RelationId::AssignLHS, {EntityId::Variable}},
    {RelationId::IfConditional, {EntityId::Variable}},
    {RelationId::WhileConditional, {EntityId::Variable}},
    {RelationId::ReadVar, {EntityId::Variable}},
    {RelationId::PrintVar, {EntityId::Variable}},
    {RelationId::CallProc, {EntityId::Procedure}}
  };
  const std::unordered_set<RelationId> RELATION_REALTIME = {
    RelationId::NextT,
    RelationId::Affects,
    RelationId::AffectsT,
    RelationId::NextBipT,
    RelationId::AffectsBip,
    RelationId::AffectsBipT,
  };
  // Stores mapping of RelationId to Relation name (as written in PQL)
  const std::unordered_map<RelationId, std::string> RELATION_ID_STRING =
    mapper::inverseOnetoOneMapper<std::string, RelationId>(RELATION_STRING_ID);

  // Stores mapping of RelationTypeId to a vector of RelationId of the particular relation type
  const std::unordered_map<RelationTypeId, std::vector<RelationId>> RELATIONS_BY_TYPE =
    mapper::inverseManytoOneMapper<RelationId, RelationTypeId>(RELATION_TYPE);

  const int NUM_RELATIONS = RELATION_STRING_ID.size();
}
