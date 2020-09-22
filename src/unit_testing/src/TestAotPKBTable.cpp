#include <catch.hpp>
#include <PKBTable.h>

#include <string>

TEST_CASE("PKB Table") {
  using namespace pkb;
  PKBTable<int, std::string> table("C1", "C2");
  table.insert(1, std::string("a"));
  table.insert(2, std::string("b"));
  REQUIRE(table.insert(2, std::string("c")) == true);
  REQUIRE(table.insert(2, std::string("c")) == false);
  table.insert(11, std::string("aa"));
  auto results = table.select();
  REQUIRE(results.size() == 4);
  REQUIRE(table.count() == 4);
  auto count = table.countWithLeft(2);
  REQUIRE(count == 2);
  REQUIRE(table.selectLeft().size()==3);
  REQUIRE(table.selectRight().size()==4);
}