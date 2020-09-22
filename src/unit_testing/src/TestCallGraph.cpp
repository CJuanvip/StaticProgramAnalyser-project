#include <catch.hpp>
#include <CallGraph.h>
#include <iostream>

TEST_CASE("Has cycle TC1") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(4);
  callGraph.addFunctionCall(1, 7);
  callGraph.addFunctionCall(3, 5);
  callGraph.addFunctionCall(5, 7);
  callGraph.addFunctionCall(7, 3);
  REQUIRE(callGraph.hasCycle() == true);
}

TEST_CASE("Has cycle TC2") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(1);
  callGraph.addFunctionCall(1, 1);
  REQUIRE(callGraph.hasCycle() == true);
}

TEST_CASE("Has cycle TC3") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(2);
  callGraph.addFunctionCall(1, 100);
  callGraph.addFunctionCall(100, 1);
  REQUIRE(callGraph.hasCycle() == true);
}


TEST_CASE("Do not have cycle TC 1") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(4);
  callGraph.addFunctionCall(1, 7);
  callGraph.addFunctionCall(3, 5);
  callGraph.addFunctionCall(5, 7);
  callGraph.addFunctionCall(3, 7);
  REQUIRE(callGraph.hasCycle() == false);
}

TEST_CASE("Do not have cycle TC 2") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(4);
  callGraph.addFunctionCall(1, 3);
  callGraph.addFunctionCall(1, 5);
  callGraph.addFunctionCall(1, 7);
  callGraph.addFunctionCall(3, 5);
  callGraph.addFunctionCall(3, 7);
  callGraph.addFunctionCall(5, 7);
  REQUIRE(callGraph.hasCycle() == false);
}

TEST_CASE("Do not have cycle TC 3") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(4);
  REQUIRE(callGraph.hasCycle() == false);
}

TEST_CASE("CallGraph Get All Caller Callee Pairs TC1") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(4);
  callGraph.addFunctionCall(1, 7);
  callGraph.addFunctionCall(3, 5);
  callGraph.addFunctionCall(5, 7);
  callGraph.addFunctionCall(3, 7);

  std::vector callerCalleePairs = callGraph.getAllCallerCalleePairs();
  // expected result:
  // (1, 7), (3, 7), (3, 5), (5, 7)
  REQUIRE(callerCalleePairs.size() == 4);
}

TEST_CASE("CallGraph Get All Caller Callee Pairs TC2") {
  using namespace pkb;
  CallGraph callGraph;
  callGraph.initialize(8);
  callGraph.addFunctionCall(1, 2);
  callGraph.addFunctionCall(1, 3);
  callGraph.addFunctionCall(2, 4);
  callGraph.addFunctionCall(2, 5);
  callGraph.addFunctionCall(3, 6);
  callGraph.addFunctionCall(3, 7);
  callGraph.addFunctionCall(5, 8);
  callGraph.addFunctionCall(6, 8);

  // dump
  std::vector callerCalleePairs = callGraph.getAllCallerCalleePairs();
  // for (const auto& pair : callerCalleePairs) {
  //   std::cout << pair.first << ", " << pair.second << "\n";
  // }

  // expected result:
  // (1, 3), (1, 2), (1, 5), (1, 4), (1, 7), (1, 6), (1, 8), (3, 7), (3, 6), (3, 8), (2, 5), (2, 4)
  // (2, 8), (5, 8), (6, 8)
  REQUIRE(callerCalleePairs.size() == 15);
}
