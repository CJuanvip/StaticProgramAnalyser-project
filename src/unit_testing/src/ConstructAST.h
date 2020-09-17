#pragma once
/// An utility class that uses a builder pattern to build a tree
/// We assume that one statement will occupy one line
/// And for if statement, else will occupy one line
/// And for while and if statement, closing bracket will occupy one line
/// Currently we do NOT support recursive expression, i.e. while/if in another while/if
#include <vector>
#include <AST.h>

using namespace std;
// class ExpressionBuilder {
// public:
//   ExpresionBuilder() : expr_(new Expression) { }

// private:
//   std::shared_ptr<Expression> expr_;
// };
class ASTBuilder {
public:
  ASTBuilder(common::StmtLst& statementList) : statements_(statementList) { }
  ASTBuilder& Read(const std::string& variableName) {
    common::Read r{common::Var{variableName}};
    common::Stmt s = r;
    statements_.push_back(s);
    return *this;
  }
  ASTBuilder& Print(const std::string& variableName) {
    common::Print p{common::Var{variableName}};
    common::Stmt s = p;
    statements_.push_back(s);
    return *this;
  }
  ASTBuilder& Assign(const std::string& variableName, const common::Expr& expr) {
    common::Assign a(common::Var{variableName}, expr);
    common::Stmt s = a;
    statements_.push_back(s);
    return *this;
  }
  ASTBuilder& Call(const std::string& procedureName) {
    common::Call c(common::Proc{procedureName});
    common::Stmt s = c;
    statements_.push_back(s);
    return *this;
  }
  ASTBuilder& While(const common::CondExpr& condExpr, const common::StmtLst& statementList) {
    common::While w(condExpr, statementList);
    common::Stmt s = w;
    statements_.push_back(s);
    return *this;
  }
  ASTBuilder& If(const common::CondExpr& condExpr, const common::StmtLst& stmtListIfTrue, const common::StmtLst& stmtListIfFalse) {
    common::If i(condExpr, stmtListIfTrue, stmtListIfFalse);
    common::Stmt s = i;
    statements_.push_back(s);
    return *this;
  }
private:
  common::StmtLst& statements_;
};

common::Program buildTestCase();

common::Program buildTestCase2();

common::Program buildTestCase3();

common::Program buildTestCase4();

common::Program buildTestCase5();

common::Program buildTestCase6();

common::Program buildTestCase7();

common::Program buildTestCase8();

common::Program buildTestCase9();