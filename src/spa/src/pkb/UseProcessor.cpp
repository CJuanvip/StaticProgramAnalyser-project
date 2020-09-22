#include <UseProcessor.h>
namespace pkb {
  UseProcessor::UseProcessor()
  : useEntityVariableMinusCall_(std::string("entity_id"), std::string("var_id"))
  , callStatementProcedure_(std::string("stmt_id"), std::string("proc_id")) { }
  void UseProcessor::insertUseEntityVariableMinusCall(unsigned nameId, unsigned varId) {
    useEntityVariableMinusCall_.insert(nameId, varId);
  }
  void UseProcessor::insertCallStatementProcedureMapping(unsigned statementId, unsigned procId) {
    callStatementProcedure_.insert(statementId, procId);
  }
  const PKBTable<unsigned, unsigned>& UseProcessor::getUseEntityVariableMinusCall() const {
    return useEntityVariableMinusCall_;
  }

  const PKBTable<unsigned, unsigned>& UseProcessor::getCallStatementProcedureMapping() const {
    return callStatementProcedure_;
  }

}