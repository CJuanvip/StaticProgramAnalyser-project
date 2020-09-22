#pragma once

#include <unordered_map>
#include <PKBTable.h>
#include <EntityId.h>
#include <string>
#include <Exception.h>
#include <NextClosureEvaluator.h>
#include <AffectEvaluator.h>
#include <AffectClosureEvaluator.h>
#include <InterproceduralNextClosureEvaluator.h>
#include <InterproceduralAffectEvaluator.h>
#include <InterproceduralAffectClosureEvaluator.h>

namespace pkb {
  class PKBTableManager {
  using StatementId = unsigned;
  using NameId = unsigned;
  using VariableId = unsigned;
  using ConstantId = unsigned;
  using ProcedureId = unsigned;

  public:
    PKBTableManager()
    : statements_(std::string("stmt_id"), std::string("type_id")),
      typesToString_(std::string("type_id"), std::string("typename")),

      assignLhs_(std::string("stmt_id"), std::string("name_id")),
      assignRhsAst_(std::string("stmt_id"), std::string("ast_string")),
      assignRhs_(std::string("stmt_id"), std::string("name_id")),

      procedureStatements_(std::string("name_id"), std::string("stmt_id")),
      statementVariables_(std::string("stmt_id"), std::string("name_id")),
      names_(std::string("name_id"), std::string("name")),
      nameTypes_(std::string("name_id"), std::string("type_id")),

      follow_(std::string("stmt_id_a"), std::string("stmt_id_b")),
      followClosure_(std::string("stmt_id_a"), std::string("stmt_id_b")),

      parent_(std::string("stmt_id_a"), std::string("stmt_id_b")),
      parentClosure_(std::string("stmt_id_a"), std::string("stmt_id_b")),

      call_(std::string("caller_id"), std::string("callee_id")),
      callClosure_(std::string("caller_id"), std::string("callee_id")),

      usesEntityVariableTable_(std::string("entity_id"), std::string("name_id")),

      modifyEntity_(std::string("name_id"), std::string("name_id")),

      next_(std::string("next_src"), std::string("next_dest")),
      nextClosure_(nextClosureEvaluator_, std::string("nextClosure_src"), std::string("nextClosure_dest")),
    
      affect_(affectEvaluator_, std::string("affect_src"), std::string("affect_dest")),

      affectClosure_(affectClosureEvaluator_, std::string("affectT_src"), std::string("affectT_dest")),

      interproceduralNext_(std::string("next_src"), std::string("next_dest")),

      interproceduralNextClosure_(interproceduralNextClosureEvaluator_, std::string("nextClosure_src"), std::string("nextClosure_dest")),

      interproceduralAffect_(interproceduralAffectEvaluator_, std::string("affect_src"), std::string("affect_dest")),

      interproceduralAffectClosure_(interproceduralAffectClosureEvaluator_, std::string("affectClosure_src"), std::string("affectClosure_dest"))
    {
      for (auto &[entityId, entityName]: common::TYPE_ID_STRING) {
        typesToString_.insert(entityId, entityName);
      }
    }

    PKBTable<NameId, std::string>& getNamesTable() {
      return names_;
    }

    PKBTable<NameId, common::EntityId>& getNameTypesTable() {
      return nameTypes_;
    }

    PKBTable<common::EntityId, std::string>& getTypestoStringTable() {
      return typesToString_;
    }

    PKBTable<StatementId, common::EntityId>& getStatementsTable() {
      return statements_;
    }
    PKBTable<unsigned, unsigned>& getFollow() {
      return follow_;
    }

    PKBTable<unsigned, unsigned>& getFollowClosure() {
      return followClosure_;
    }

    PKBTable<unsigned, unsigned>& getParent() {
      return parent_;
    }

    PKBTable<unsigned, unsigned>& getParentClosure() {
      return parentClosure_;
    }

    PKBTable<NameId, NameId>& getUsesEntityVariableTable() {
      return usesEntityVariableTable_;
    }

    PKBTable<NameId, NameId>& getModifyEntity() {
      return modifyEntity_;
    }

    PKBTable<NameId, NameId>& getCall() {
      return call_;
    }

    PKBTable<NameId, NameId>& getCallClosure() {
      return callClosure_;
    }

    PKBTable<NameId, NameId>& getNext() {
      return next_;
    }

    RealtimePKBTable<NameId, NameId>& getNextClosure() {
      return nextClosure_;
    }

    RealtimePKBTable<NameId, NameId>& getAffect() {
      return affect_;
    }

    RealtimePKBTable<NameId, NameId>& getAffectClosure() {
      return affectClosure_;
    }

    PKBTable<NameId, NameId>& getInterproceduralNext() {
      return interproceduralNext_;
    }

    RealtimePKBTable<NameId, NameId>& getInterproceduralNextClosure() {
      return interproceduralNextClosure_;
    }

    RealtimePKBTable<NameId, NameId>& getInterproceduralAffect() {
      return interproceduralAffect_;
    }

    RealtimePKBTable<NameId, NameId>& getInterproceduralAffectClosure() {
      return interproceduralAffectClosure_;
    }

    PKBTable<StatementId, NameId>& getAssignLhs() {
      return assignLhs_;
    }

    PKBTable<StatementId, NameId>& getAssignRhs() {
      return assignRhs_;
    }

    PKBTable<StatementId, std::string>& getAssignRhsAst() {
      return assignRhsAst_;
    }

    PKBTable<StatementId, NameId>& getStatementVariables() {
      return statementVariables_;
    }

    PKBTable<NameId, StatementId>& getProcedureStatements() {
      return procedureStatements_;
    }

    void prepareForRealtimeEvaluation(ControlFlowGraph& cfg, CallGraph& callGraph) {
      nextClosureEvaluator_.initialize(*this, cfg);
      affectEvaluator_.initialize(*this, cfg);
      affectClosureEvaluator_.initialize(*this);
      interproceduralNextClosureEvaluator_.initialize(*this, cfg);
      interproceduralAffectEvaluator_.initialize(*this, cfg, callGraph); 
      interproceduralAffectClosureEvaluator_.initialize(*this, cfg, callGraph);
    }

    void clearCache() {
      nextClosureEvaluator_.reset();
      nextClosure_.clear();
      affectEvaluator_.reset();
      affect_.clear();
      affectClosureEvaluator_.reset();
      affectClosure_.clear();
      interproceduralNextClosureEvaluator_.reset();
      interproceduralNextClosure_.clear();
      interproceduralAffectEvaluator_.reset();
      interproceduralAffect_.clear();
      interproceduralAffectClosureEvaluator_.reset();
      interproceduralAffectClosure_.clear();
    }
  private:

    PKBTable<StatementId, common::EntityId> statements_;
    PKBTable<common::EntityId, std::string> typesToString_;

    PKBTable<StatementId, NameId> assignLhs_;
    PKBTable<StatementId, std::string> assignRhsAst_;
    PKBTable<StatementId, NameId> assignRhs_; // Appearances of a certain name in a statement's RHS
    /*
     * e.g. stmt1. a = b+c
     * assignLhs contains 1 -> a
     * assignRhs contains 1 -> b and 1 -> c
     * assignRhsAst contains 1 -> <ast_expr_of_a=b+c>
     */

    PKBTable<NameId, StatementId> procedureStatements_; // contains procId -> stmtId iff that proc contains that stmt
    PKBTable<StatementId, NameId> statementVariables_; // contains stmtId -> varId iff that stmt contains that var
    PKBTable<NameId, std::string> names_;

    PKBTable<NameId, common::EntityId> nameTypes_;

    PKBTable<StatementId, StatementId> follow_;
    PKBTable<StatementId, StatementId> followClosure_;

    PKBTable<StatementId, StatementId> parent_;
    PKBTable<StatementId, StatementId> parentClosure_;

    PKBTable<NameId, NameId> call_;
    PKBTable<NameId, NameId> callClosure_;

    PKBTable<NameId, NameId> usesEntityVariableTable_;

    // name_id->var_id
    PKBTable<NameId, NameId> modifyEntity_;

    PKBTable<NameId, NameId> next_;

    NextClosureEvaluator nextClosureEvaluator_;
    RealtimePKBTable<NameId, NameId> nextClosure_;

    AffectEvaluator affectEvaluator_;
    RealtimePKBTable<NameId, NameId> affect_;

    AffectClosureEvaluator affectClosureEvaluator_;
    RealtimePKBTable<NameId, NameId> affectClosure_;

    PKBTable<NameId, NameId> interproceduralNext_;

    InterproceduralNextClosureEvaluator interproceduralNextClosureEvaluator_;
    RealtimePKBTable<NameId, NameId> interproceduralNextClosure_;

    InterproceduralAffectEvaluator interproceduralAffectEvaluator_;
    RealtimePKBTable<NameId, NameId> interproceduralAffect_;
    
    InterproceduralAffectClosureEvaluator interproceduralAffectClosureEvaluator_;
    RealtimePKBTable<NameId, NameId> interproceduralAffectClosure_;
  };
}
