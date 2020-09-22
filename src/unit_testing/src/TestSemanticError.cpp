#include <catch.hpp>
#include <ConstructAST.h>
#include <DesignExtractor.h>

TEST_CASE("Test cyclic call") {
  using namespace common;
  using namespace ast;
  StmtLst s1;
  ASTBuilder(s1).Call("a");
  ProcDef p("a", s1);
  Program prog{p};
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(prog);
  CHECK_THROWS(designExtractor.extract(), ERROR_MESSAGE_CYCLIC_CALL);
}

TEST_CASE("Test Duplicate Procedure Name") {
  using namespace ast;
  using namespace common;
  StmtLst s1;
  ProcDef p1{"a", s1};
  StmtLst s2;
  ProcDef p2{"a", s2};
  Program prog{p1, p2};
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(prog);
  CHECK_THROWS(designExtractor.extract(), ERROR_MESSAGE_DUPLICATE_PROC_NAME);
}

TEST_CASE("Test call to nonexistent procedure") {
  using namespace ast;
  using namespace common;
  StmtLst s1;
  ASTBuilder(s1).Call("a");
  ProcDef p{"b", s1};
  Program prog{p};
  pkb::PKBTableManager mgr;
  DesignExtractor designExtractor(mgr);
  designExtractor.initialize(prog);
  CHECK_THROWS(designExtractor.extract(), ERROR_MESSAGE_NON_EXISTENT_PROC_NAME);
}