#include <StatementInformationExtractor.h>
#include <vector>
#include <string>
#include <SPAAssert.h>
namespace ast {
  using namespace common;
  namespace {
  unsigned getNameId(const std::string& s, const pkb::PKBTable<unsigned, std::string>& namesTable) {
    const auto& filtered = namesTable.filterRight(s);
    SPA_ASSERT( filtered.size() == 1);
    return *filtered.begin();
  }
  struct ExpressionNameIdCollector {
    ExpressionNameIdCollector(const pkb::PKBTable<unsigned, std::string>& namesTable)
    : namesTable_(namesTable) { }
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
      std::string s = o.name;
      collectedNameIds_.push_back(getNameId(s, namesTable_));
    }
    void operator() (const Const& o) {
      std::string s = o.value;
      collectedNameIds_.push_back(getNameId(s, namesTable_));
    }

    const std::vector<unsigned>& getCollectedNameIds() const {
      return collectedNameIds_;
    }
    void clearCollectedNameIds() {
      collectedNameIds_.clear();
    }
  private:
    const pkb::PKBTable<unsigned, std::string>& namesTable_;
    std::vector<unsigned> collectedNameIds_;
  };

  struct ConditionalExpressionNameIdCollector {
    ConditionalExpressionNameIdCollector(const pkb::PKBTable<unsigned, std::string>& namesTable)
    : namesTable_(namesTable)
    , collector_(namesTable_) { }
    void operator() (const Not& o) {
      std::visit(*this,*o.left);
    }
    void operator() (const And& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const Or& o) {
      std::visit(*this,*o.left);
      std::visit(*this,*o.right);
    }
    void operator() (const RelExpr& o) {
      std::visit([this](auto const& e) {
        std::visit(this->collector_, *e.left);
        std::visit(this->collector_, *e.right);
        const auto& collectorsCollection = collector_.getCollectedNameIds();
        collectedNameIds_.insert(collectedNameIds_.end(),
        collectorsCollection.begin(), collectorsCollection.end());
        collector_.clearCollectedNameIds();
      }, o);
    }
    const std::vector<unsigned>& getCollectedNameIds() const {
      return collectedNameIds_;
    }
    void clearCollectedNameIds() {
      collectedNameIds_.clear();
    }
  private:
    const pkb::PKBTable<unsigned, std::string>& namesTable_;
    std::vector<unsigned> collectedNameIds_;
    ExpressionNameIdCollector collector_;
  };
  } //anonymous
  void StatementInformationExtractor::operator() (const Read& o) {
    statementVariableTable_.insert(o.statementId, getNameId(toString(o.variable), namesTable_));
  }

  void StatementInformationExtractor::operator() (const Print& o) {
    statementVariableTable_.insert(o.statementId, getNameId(toString(o.variable), namesTable_));
  }

  void StatementInformationExtractor::operator() (const Call& o) {
    statementVariableTable_.insert(o.statementId, getNameId(toString(o.procedure), namesTable_));
  }

  void StatementInformationExtractor::operator() (const While& o) {
    ConditionalExpressionNameIdCollector condExprCollector(namesTable_);
    std::visit(condExprCollector, o.condition);
    const auto& collectedCondExpr = condExprCollector.getCollectedNameIds();
    for(const auto nameId: collectedCondExpr) {
      statementVariableTable_.insert(o.statementId, nameId);
    }
    for(const auto& s: o.statements) {
      std::visit(*this, s);
    }
  }

  void StatementInformationExtractor::operator() (const If& o) {
    ConditionalExpressionNameIdCollector condExprCollector(namesTable_);
    std::visit(condExprCollector, o.condition);
    const auto& collectedCondExpr = condExprCollector.getCollectedNameIds();
    for(const auto nameId: collectedCondExpr) {
      statementVariableTable_.insert(o.statementId, nameId);
    }
    for(auto& s: o.thenStatements) {
      std::visit(*this, s);
    }
    for(auto& s: o.elseStatements) {
      std::visit(*this, s);
    }
  }

  void StatementInformationExtractor::operator() (const Assign& o) {
    ExpressionNameIdCollector collector(namesTable_);
    std::visit(collector, *o.value);
    const auto& collected = collector.getCollectedNameIds();
    auto lhsNameId = getNameId(toString(o.variable), namesTable_);
    // StatementVariable
    statementVariableTable_.insert(o.statementId, lhsNameId);
    for(const auto& nameId: collected) {
      statementVariableTable_.insert(o.statementId, nameId);
    }
    // Assign
    assignLhsTable_.insert(o.statementId, lhsNameId);
    for(const auto& nameId: collected) {
      assignRhsTable_.insert(o.statementId, nameId);
    }
    assignRhsAstTable_.insert(o.statementId, toString(*o.value));
  }
}
