#include <catch.hpp>
#include <PKBTableManager.h>
#include <DesignExtractor.h>
#include <ConstructAST.h>
TEST_CASE("Extract program with no call") {
  using namespace ast;
  using namespace common;
  Program program = buildTestCase6();
  pkb::PKBTableManager mgr;
  ast::DesignExtractor extractor(mgr);
  extractor.initialize(program);
  extractor.extract();
}