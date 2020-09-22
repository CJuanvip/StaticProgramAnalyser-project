#include <CFGExtractor.h>
namespace ast {
  void extractCFG(pkb::ControlFlowGraph& cfg, const common::Program& program, pkb::PKBTableManager& mgr) {
    cfg.initialize(mgr.getStatementsTable().count());
    for (const auto& procedure: program) {
      cfg.process(procedure.statements);
    }
    cfg.resolveCallTasks(mgr);
  }
}