#pragma once
#include <string>
#include <unordered_map>
#include "parser/AST.h"

namespace query_eval {
  using namespace common;

  struct ExpressionNameCollector {
    ExpressionNameCollector() { }
    void operator() (const Add& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Sub& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Mul& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Div& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Mod& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Var& o) {
      nameFreqTable_[o.name]++;
    }
    void operator() (const Const& o) {
      nameFreqTable_[o.value]++;
    }

    const std::unordered_map<std::string, unsigned>& getCollectedNames() const {
      return nameFreqTable_;
    }
    void clearCollectedNames() {
      nameFreqTable_.clear();
    }
  private:
    std::unordered_map<std::string, unsigned> nameFreqTable_;
  };
}
