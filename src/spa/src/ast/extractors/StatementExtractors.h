#pragma once
#include <vector>
#include <memory>
#include <AST.h>
#include <CallGraph.h>
#include <PKBTableManager.h>
namespace ast {
  class StatementVisitorBase {
  public:
    virtual void operator()(const common::Read&) = 0;
    virtual void operator()(const common::Print&) = 0;
    virtual void operator()(const common::Assign&) = 0;
    virtual void operator()(const common::Call&) = 0;
    virtual void operator()(const common::While&) = 0;
    virtual void operator()(const common::If&) = 0;
  };
  class CombinedStatementVisitor {
  public:
    CombinedStatementVisitor(std::vector<std::shared_ptr<StatementVisitorBase>>& visitors) :
        visitors_(visitors) { }
    template <class T>
    void operator() (const T& s) {
      for(auto& ptr: visitors_) {
        ptr->operator()(s);
      }
    }
  private:
    std::vector<std::shared_ptr<StatementVisitorBase>> visitors_;
  };
  
  void extractStatementsInformation(const common::ProcDef&, pkb::CallGraph&, pkb::PKBTableManager&);
}