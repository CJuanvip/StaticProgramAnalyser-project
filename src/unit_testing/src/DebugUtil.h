// An utility class that helps to print StatementId

#include <vector>
#include <AST.h>
using namespace std;
using namespace common;

struct StatementIdPrinter {
  void operator() (const While& o) {
    std::cout << o.statementId << std::endl;
    for(const auto& stmt: o.statements) {
      std::visit(*this, stmt);
    }
  }
  void operator() (const If& o) {
    std::cout << o.statementId << std::endl;
    for(const auto& stmt: o.thenStatements) {
      std::visit(p, stmt);
    }
    for(const auto& stmt: o.elseStatements) {
      std::visit(*this, stmt);
    }
  }
  void operator() (const Call& o) {
    std::cout << o.statementId << std::endl;
  }
  void operator() (const Read& o) {
    std::cout << o.statementId << std::endl;
  }
  void operator() (const Print& o) {
    std::cout << o.statementId << std::endl;
  }
  void operator() (const Assign& o) {
    std::cout << o.statementId << std::endl;
  }
};

// usage
/*
  for (const auto& proc: prog) {
    for (const auto& stmt: proc.statements) {
      std::visit(StatementIdPrinter(), stmt);
    }
  }
*/
  