#pragma once
#include <StatementExtractors.h>
#include <PKBTableManager.h>
namespace ast {
  /// A visitor that extracts information AFTER Var/Const has its nameId populated
  struct StatementInformationExtractor : StatementVisitorBase {
    StatementInformationExtractor(pkb::PKBTableManager& mgr)
    : namesTable_(mgr.getNamesTable())
    , statementVariableTable_(mgr.getStatementVariables())
    , assignLhsTable_(mgr.getAssignLhs())
    , assignRhsTable_(mgr.getAssignRhs())
    , assignRhsAstTable_(mgr.getAssignRhsAst()) { }
    void operator() (const common::Read& o) override;

    void operator() (const common::Print& o) override;

    void operator() (const common::Call& o) override;

    void operator() (const common::While& o) override;

    void operator() (const common::If& o) override;

    void operator() (const common::Assign& o) override;
  private:
    const pkb::PKBTable<unsigned, std::string>& namesTable_;
    pkb::PKBTable<unsigned, unsigned>& statementVariableTable_;
    pkb::PKBTable<unsigned, unsigned>& assignLhsTable_;
    pkb::PKBTable<unsigned, unsigned>& assignRhsTable_;
    pkb::PKBTable<unsigned, std::string>& assignRhsAstTable_;
  };
}