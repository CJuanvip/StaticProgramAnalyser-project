#pragma once
#include <PKBTable.h>
namespace pkb {
  struct UseProcessor {
    UseProcessor();
    void insertUseEntityVariableMinusCall(unsigned, unsigned);
    void insertCallStatementProcedureMapping(unsigned, unsigned);
    const PKBTable<unsigned, unsigned>& getUseEntityVariableMinusCall() const;

    const PKBTable<unsigned, unsigned>& getCallStatementProcedureMapping() const;
  private:
    PKBTable<unsigned, unsigned> useEntityVariableMinusCall_;
    PKBTable<unsigned, unsigned> callStatementProcedure_;
  };
} 