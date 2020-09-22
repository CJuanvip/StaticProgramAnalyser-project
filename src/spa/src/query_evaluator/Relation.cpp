#include "Relation.h"

namespace query_eval {

  RelationId Relation::getRelationId() const {
    return id_;
  }

  RelationTypeId Relation::getRelationType() const {
    return RELATION_TYPE.at(id_);
  }


  EntityId Relation::getLeftWildcardType() const {
    RelationTypeId relationType = getRelationType();
    if (relationType == RelationTypeId::Stmt_Stmt) return EntityId::Statement;
    else if (relationType == RelationTypeId::Proc_Proc) return EntityId::Procedure;
    return EntityId::Wildcard;
  }

  EntityId Relation::getRightWildcardType() const {
    RelationTypeId relationType = getRelationType();
    if (relationType == RelationTypeId::Stmt_Stmt) return EntityId::Statement;
    else if (relationType == RelationTypeId::Proc_Proc) return EntityId::Procedure;
    else if (relationType == RelationTypeId::StmtProc_Var) return EntityId::Variable;
    return EntityId::Wildcard;
  }
}
