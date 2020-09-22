#pragma once
#include <string>
#include <algorithm>
#include "Relation.h"
#include "parser/AST.h"
#include "common/EntityId.h"
#include "common/RelationId.h"

namespace query_eval {
  using namespace common;
  class Argument {
  public:
    Argument (){}
    Argument (const std::string& content);

    // Declares implicit typecast to strings
    operator std::string() const { return content_; }
    operator std::string&() { return content_; }
    operator const std::string&() const { return content_; }

    // Checks if argument is a wildcard (eg: _)
    bool isWildcard() const { return wildcard_; }
    // Checks if argument is a numeric constant (eg INTEGER)
    bool isNumericConstant() const { return numericConstant_; }
    // Checks if argument is a string constant (eg: "IDENT")
    bool isStringConstant() const { return stringConstant_; }
    // Checks if argument is a constant (numeric/string)
    bool isConstant() const { return numericConstant_ || stringConstant_; }
    // Checks if argument is an entity (eg: IDENT)
    bool isEntity() const { return entity_; }
    // Checks if argument is one of the above types
    bool isValid() const { return wildcard_ || numericConstant_ || stringConstant_ || entity_; };

  private:
    std::string content_;
    bool wildcard_ = false;
    bool numericConstant_ = false;
    bool stringConstant_ = false;
    bool entity_ = false;
  };

  struct SuchThatClause {
    Relation relation;
    Argument leftArg;
    Argument rightArg;

    SuchThatClause(const std::string &relation_, const std::string &leftArg_, const std::string &rightArg_);
    SuchThatClause(const SuchThat &suchThat);

    bool isValid() const;
  };
}
