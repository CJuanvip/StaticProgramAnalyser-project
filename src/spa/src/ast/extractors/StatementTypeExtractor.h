#pragma once
#include <AST.h>
#include <PKBTableManager.h>
#include <StatementExtractors.h>
#include <EntityId.h>
namespace ast {
  struct StatementTypeExtractor : StatementVisitorBase {
    StatementTypeExtractor(pkb::PKBTable<unsigned, common::EntityId>& table)
    : table_(table) { }
    void operator() (const common::Read& o) override {
      table_.insert(o.statementId, common::EntityId::Read);
    }

    void operator() (const common::Print& o) override {
      table_.insert(o.statementId, common::EntityId::Print);
    }

    void operator() (const common::Call& o) override {
      table_.insert(o.statementId, common::EntityId::Call);
    }

    void operator() (const common::While& o) override {
      table_.insert(o.statementId,common::EntityId::While);
      for(const auto& s: o.statements) {
        std::visit(*this, s);
      }
    }

    void operator() (const common::If& o) override {
      table_.insert(o.statementId, common::EntityId::If);
      for(auto& s: o.thenStatements) {
        std::visit(*this, s);
      }
      for(auto& s: o.elseStatements) {
        std::visit(*this, s);
      }
    }

    void operator() (const common::Assign& o) override {
      table_.insert(o.statementId, common::EntityId::Assign);
    }
  private: 
    pkb::PKBTable<unsigned, common::EntityId>& table_; // statementsTable
  };
}