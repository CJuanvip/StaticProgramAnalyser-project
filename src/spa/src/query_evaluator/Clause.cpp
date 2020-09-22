#include "Clause.h"
#include "Visitor.h"

namespace query_eval {

  Argument::Argument (const std::string& content) {
    content_ = content;
    if (content.size() == 0) return;
    if (content == "_") {
      wildcard_ = true;
      return;
    }
    // Numeric constants have all characters as digits
    numericConstant_ = std::all_of(content.begin(), content.end(), isdigit);
    // IDENT is all alnum except the first is letters
    entity_ = std::all_of(content.begin(), content.end(), isalnum) &&
      isalpha(content[0]);

    // String constants are surrounded by " "
    if (content.size() > 2) {
      stringConstant_ = content[0] == '"' &&
        content.back() == '"' &&
        std::all_of(content.begin()+1, content.end()-1, isalnum) &&
        isalpha(content[1]);
    }
    if (stringConstant_) {
      content_ = content.substr(1, content.size()-2);
    }
  }

  SuchThatClause::SuchThatClause(const std::string &relation_, const std::string &leftArg_, const std::string &rightArg_):
    relation(relation_),
    leftArg(leftArg_),
    rightArg(rightArg_)
  { }

  SuchThatClause::SuchThatClause(const SuchThat &suchThat) {
    std::visit(visitor {
      [&](const auto &r) {
        relation = r.type;
        leftArg = toString(r.left);
        rightArg = toString(r.right);
      },
      [&](const StmtEntEntRelation& r) {
        relation = r.type;
        leftArg = std::visit(visitor{
          [&](const auto &e) { return toString(e); }
        }, r.left);
        rightArg = toString(r.right);
      }
    }, suchThat);
  }

  bool SuchThatClause::isValid() const {
    if (!leftArg.isValid() || !rightArg.isValid()) return false;
    if (relation.getRelationType() == RelationTypeId::Stmt_Stmt) {
      if (leftArg.isStringConstant() || rightArg.isStringConstant()) return false;
    } else if (relation.getRelationType() == RelationTypeId::StmtProc_Var) {
      if (leftArg.isWildcard() || rightArg.isNumericConstant()) return false;
    } else if (relation.getRelationType() == RelationTypeId::Proc_Proc) {
      if (leftArg.isNumericConstant() || rightArg.isNumericConstant()) return false;
    } else {
      return false;
    }
    return true;
  }
}
