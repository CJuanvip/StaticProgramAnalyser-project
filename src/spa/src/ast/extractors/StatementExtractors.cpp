#include <StatementExtractors.h>
#include <StatementTypeExtractor.h>
#include <NameExtractor.h>
#include <StatementInformationExtractor.h>
#include <CallExtractor.h>
namespace ast {
  void extractStatementsInformation(const common::ProcDef& procedure, 
                                    pkb::CallGraph& callGraph,
                                    pkb::PKBTableManager& mgr) {
    std::vector<std::shared_ptr<StatementVisitorBase>> visitors;
    visitors.push_back(
        std::shared_ptr<StatementVisitorBase>(
            new StatementTypeExtractor(mgr.getStatementsTable()))); 
    visitors.push_back(
        std::shared_ptr<StatementVisitorBase>(
            new NameExtractor(mgr)));
    visitors.push_back(
        std::shared_ptr<StatementVisitorBase>(
            new StatementInformationExtractor(mgr)));
    visitors.push_back(
        std::shared_ptr<StatementVisitorBase>(
            new CallExtractor(mgr, callGraph)));
    CombinedStatementVisitor combinedVisitor{visitors};
    for(const auto& statement: procedure.statements) {
      std::visit(combinedVisitor, statement);
    }
    if (callGraph.hasCycle()) {
      throw SemanticError(ERROR_MESSAGE_CYCLIC_CALL);
    }
  }
}