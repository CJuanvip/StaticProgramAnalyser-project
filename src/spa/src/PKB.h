#pragma once

#include <AST.h>
#include <DesignExtractor.h>

namespace pkb {

class PKB {
public:
  PKB() : extractor_(pkbTableManager_) { }
  void parse(common::Program& program) {
    extractor_.initialize(program);
    extractor_.extract();
  }

  pkb::PKBTableManager& getPKBTableManager() {
    return pkbTableManager_;
  }
private:
  pkb::PKBTableManager pkbTableManager_;
  ast::DesignExtractor extractor_;

};

}
