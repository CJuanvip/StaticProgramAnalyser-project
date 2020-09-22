#include <NameExtractor.h>
#include <ASTToString.h>
#include <SPAAssert.h>
#include <EntityId.h>
#include <Exception.h>
namespace ast {
  namespace {
  using namespace common;
  template <class T>
  void extractName(const T& t, pkb::PKBTableManager& mgr, common::EntityId type) {
    auto& namesTable = mgr.getNamesTable();
    auto& nameTypesTable = mgr.getNameTypesTable();
    const auto& nameIds = namesTable.filterRight(toString(t));
    if (!nameIds.empty()) {
      SPA_ASSERT (nameIds.size() == 1);
      auto nameId = *nameIds.begin();
      const auto& nameTypes = nameTypesTable.filterLeft(nameId);
      SPA_ASSERT (!nameTypes.empty());
      // if we do not have such type, add the type
      if (nameTypes.find(type) == nameTypes.end()) {
        nameTypesTable.insert(nameId, type);
      }
    } else {
      // add to namesTable
      auto nameId = namesTable.count() + 1;
      namesTable.insert(nameId, toString(t));
      nameTypesTable.insert(nameId, type);
    }
  }
  struct ExpressionNameExtractor {
    ExpressionNameExtractor(pkb::PKBTableManager& mgr)
    : namesTable_(mgr.getNamesTable())
    , nameTypesTable_(mgr.getNameTypesTable()) { }
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
      populate(s, EntityId::Variable);
    }
    void operator() (const Const& o) {
      std::string s = o.value;
      populate(s, EntityId::Constant);
    }
  private:
    void populate(const std::string& s, common::EntityId type) {
      const auto& filtered = namesTable_.filterRight(s);
      if (filtered.empty()) {
        auto nameId = namesTable_.count() + 1;
        namesTable_.insert(nameId, s);
        nameTypesTable_.insert(nameId, type);
      } else {
        SPA_ASSERT(filtered.size() == 1);
        auto nameId = *filtered.begin();
        nameTypesTable_.insert(nameId, type);
      }
    }
    pkb::PKBTable<unsigned, std::string>& namesTable_;
    pkb::PKBTable<unsigned, EntityId>& nameTypesTable_;
  };// Add,Sub,Mul,Div,Mod,Var,Const
  struct ConditionalExpressionNameExtractor {
    ConditionalExpressionNameExtractor(pkb::PKBTableManager& mgr)
    : namesTable_(mgr.getNamesTable())
    , nameTypesTable_(mgr.getNameTypesTable())
    , expressionNameExtractor_(mgr) { }
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
        std::visit(this->expressionNameExtractor_, *e.left);
        std::visit(this->expressionNameExtractor_, *e.right);
      }, o);
    }
  private:
    void populate(const std::string& s, common::EntityId type) {
      const auto& filtered = namesTable_.filterRight(s);
      if (filtered.empty()) {
        auto nameId = namesTable_.count() + 1;
        namesTable_.insert(nameId, s);
        nameTypesTable_.insert(nameId, type);
      } else {
        SPA_ASSERT(filtered.size() == 1);
        auto nameId = *filtered.begin();
        nameTypesTable_.insert(nameId, type);
      }
    }
    pkb::PKBTable<unsigned, std::string>& namesTable_;
    pkb::PKBTable<unsigned, common::EntityId>& nameTypesTable_;
    ExpressionNameExtractor expressionNameExtractor_;
  }; // Not,And,Or,RelExpr

  } // anonymous

  void extractName(const Var& variable, pkb::PKBTableManager& mgr) {
    extractName(variable, mgr, EntityId::Variable);
  }
  void extractName(const Const& constant, pkb::PKBTableManager& mgr) {
      extractName(constant, mgr, EntityId::Constant);
  }
  void extractNames(const Expr& expr, pkb::PKBTableManager& mgr) {
    ExpressionNameExtractor extractor(mgr);
    std::visit(extractor, expr);
  }
  void extractNames(const CondExpr& condExpr, pkb::PKBTableManager& mgr) {
    ConditionalExpressionNameExtractor extractor(mgr);
    std::visit(extractor, condExpr);
  }
  void NameExtractor::verifyExistence(const Proc& procedureName) {
    auto& namesTable = mgr_.getNamesTable();
    auto& nameTypesTable = mgr_.getNameTypesTable();
    const auto& filtered = namesTable.filterRight(procedureName.name);
    if (filtered.size() == 0) {
      throw SemanticError(ERROR_MESSAGE_NON_EXISTENT_PROC_NAME);
    }
    SPA_ASSERT(filtered.size() == 1);
    auto procId = *filtered.begin();
    const auto& types = nameTypesTable.filterLeft(procId);
    if (types.find(EntityId::Procedure) == types.end()) {
      throw SemanticError(ERROR_MESSAGE_NON_EXISTENT_PROC_NAME);
    }
  }
}
