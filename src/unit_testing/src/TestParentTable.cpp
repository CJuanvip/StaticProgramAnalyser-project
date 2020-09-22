#include <catch.hpp>
#include <PKBTableManager.h>
#include <ConstructAST.h>
#include <StatementIdNormalizer.h>
#include <ParentExtractor.h>

TEST_CASE("Test Parent") {

  using namespace ast;
  using namespace common;
  Const c{"1"};
  // A filler for all expression
  // Since we are testing parent, we do not care about the meaning of the expresssion

  /*
  procA {
    read i        #1
    while () {    #2
      read i      #3
      while() {   #4
        read x    #5
        y = c     #6
        print z   #7
      }           #
      read i      #8
    }             #
    print i       #9
  }
  */
  Expr expr = c;
  StmtLst whileStatementList;
  ASTBuilder whileStatementListBuilder(whileStatementList);
  whileStatementListBuilder.Read("x")
                  .Assign("y", c)
                  .Print("z");
  REQUIRE(whileStatementList.size() == 3);

  StmtLst statementList;
  ASTBuilder astBuilder(statementList);
  CondExpr condExpr;
  astBuilder.Read("i")
            .While(condExpr, whileStatementList)
            .Print("i");
  REQUIRE(statementList.size() == 3);

  StmtLst statementList2;
  ASTBuilder astBuilder2(statementList2);
  CondExpr condExpr2;
  astBuilder2.Read("i")
             .While(condExpr2, statementList)
             .Print("i");
  REQUIRE(statementList2.size() == 3);

  ProcDef procedureA("procA", statementList2);
  Program program{procedureA};

  pkb::PKBTableManager mgr;
  normalizeStatementId(program, mgr);
  for(const auto& procedure: program) {
    extractParent(procedure,mgr);
  }

  // std::cout << "Parent table dump\n";
  // mgr.getParent().dump();
  // std::cout << "________"<<std::endl;
  // Expected parent relations:
  // (4, 5), (4, 6), (4, 7)
  // (2, 3), (2, 4), (2, 8)
  REQUIRE(mgr.getParent().select().size() == 6);

  // std::cout << "ParentClosure table dump\n";
  // mgr.getParentClosure().dump();
  // std::cout << "________"<<std::endl;
  // Expected parentClosure relations:
  // (4, 5), (4, 6), (4, 7)
  // (2, 3), (2, 4), (2, 8)
  // (2, 5), (2, 6), (2, 7)
  REQUIRE(mgr.getParentClosure().select().size() == 9);
}

TEST_CASE("A more complicated program") {
  using namespace ast;
  auto program = buildTestCase2();
  pkb::PKBTableManager mgr;
  normalizeStatementId(program, mgr);
  for(const auto& procedure: program) {
    extractParent(procedure,mgr);
  }

  // verify
  // std::cout << "Parent table dump"<<std::endl;
  // mgr.getParent().dump();
  // std::cout << "________"<<std::endl;

  // expected parent relations:
  // (5, 6), (5, 7), (5, 8)
  // (8, 9), (8, 10)
  // (10, 11), (10, 12)
  REQUIRE(mgr.getParent().select().size() == 7);


  // std::cout << "ParentClosure table dump\n";
  // mgr.getParentClosure().dump();
  // std::cout << "________"<<std::endl;
  // Expected parentClosure relations:
  // (5, 6), (5, 7), (5, 8)
  // (8, 9), (8, 10)
  // (10, 11), (10, 12)
  // (5, 9), (5, 10), (5, 11), (5, 12)
  // (8, 11), (8, 12)
  REQUIRE(mgr.getParentClosure().select().size() == 13);

}
