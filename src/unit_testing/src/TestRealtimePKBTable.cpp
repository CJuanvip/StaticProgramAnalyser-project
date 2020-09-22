#include <catch.hpp>
#include <PKBTable.h>

using namespace pkb;
namespace {
  void verify(const std::vector<PKBResult<unsigned, unsigned>>& results) {
    std::vector<unsigned> fib;
    unsigned prev = 1;
    unsigned curr = 1;
    while(prev < 10000) {
      fib.push_back(curr);
      unsigned next = prev + curr;
      prev = curr;
      curr = next;
    }
    REQUIRE(results.size() == fib.size() - 1);
    size_t first = 0;
    size_t second = 1;
    for (const auto& result: results) {
      REQUIRE(result.getLeft() == fib[first++]);
      REQUIRE(result.getRight() == fib[second++]);
    }
  }
  class FibonacciEvaluator : public RealtimeEvaluator<unsigned, unsigned> {
  public:
    FibonacciEvaluator() = default;
    // evaluate the next 10 fibanocci sequence not present in table; always return false
    bool select(PKBTable<unsigned, unsigned>& table) override {
      addToTable(table);
      return true;
    }
    // evaluate until left content <= left; always return false
    bool filterLeft(const unsigned& left, PKBTable<unsigned, unsigned>& table) override {
      addToTable(table);
      return true;
    }
    bool filterRight(const unsigned& right, PKBTable<unsigned, unsigned>& table) override {
      addToTable(table);
      return true;
    }
    bool contains(const unsigned&, const unsigned&, PKBTable<unsigned, unsigned>& table) {
      addToTable(table);
      return true;
    }
  private:
    void addToTable(PKBTable<unsigned, unsigned>& table) {
      REQUIRE(allowCalled_);
      allowCalled_ = false;
      while(currFibValue_ < 10000) {
        unsigned nextFibValue = prevFibValue_ + currFibValue_;
        table.insert(currFibValue_, nextFibValue);
        prevFibValue_ = currFibValue_;
        currFibValue_ = nextFibValue;
      }
    }
    unsigned prevFibValue_ = 1;
    unsigned currFibValue_ = 1;
    bool allowCalled_ = true;
  };
}
TEST_CASE("Test Delayed Evaluation") {
  FibonacciEvaluator evaluator;
  RealtimePKBTable<unsigned, unsigned> table(evaluator, std::string("prev"), std::string("next"));
  const auto ans = table.select();
  REQUIRE(table.contains(4181, 6765) == true);
  REQUIRE(table.contains(9999, 10000) == false);
  REQUIRE(table.filterRight(4181).size() == 1);
  REQUIRE(table.filterLeft(1).size() == 1);
  verify(table.select());
}
