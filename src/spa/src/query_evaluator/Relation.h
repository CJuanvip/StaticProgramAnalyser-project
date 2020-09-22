#pragma once
#include <string>
#include <SPAAssert.h>
#include <vector>
#include <unordered_map>
#include "common/RelationId.h"
#include "common/EntityId.h"

namespace query_eval {
  using namespace common;

  class Relation {
    public:
    Relation(){}
    Relation (const std::string& str) {
      SPA_ASSERT(RELATION_STRING_ID.count(str) == 1);
      id_ = RELATION_STRING_ID.at(str);
    }
    Relation (const RelationId& relationId) { id_ = relationId; }

    // Declares implicit typecast to strings
    operator std::string() const { return RELATION_ID_STRING.at(id_); }
    operator const std::string&() const { return RELATION_ID_STRING.at(id_); }

    operator RelationId() const { return getRelationId(); }

    // Checks if relation is a valid one-
    RelationId getRelationId() const;
    RelationTypeId getRelationType() const;
    EntityId getLeftWildcardType() const;
    EntityId getRightWildcardType() const;

    private:
    RelationId id_;

  };

}
