#include <catch.hpp>
#include <query_evaluator/ResultTable.h>
#include <random>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
using ResultTable = query_eval::ResultTable;
using TableData = std::pair<std::vector<query_eval::SchemaType>, std::vector<query_eval::RowType>>;

ResultTable generateResultTable(const TableData &data) {
  ResultTable result(data.second);
  return result.reschema(data.first);
}

void checkEqual(const ResultTable &result, const TableData &data) {
  std::unordered_map<query_eval::SchemaType, int> schemaMapping;
  auto schema = result.schema();
  REQUIRE(result.size() == data.second.size());
  if (result.size() == 0) return;
  REQUIRE(schema.size() == data.first.size());
  for (size_t i = 0; i < schema.size(); ++i) {
    schemaMapping[schema[i]] = i;
  }
  for (auto &schema: data.first) {
    REQUIRE(schemaMapping.count(schema) == 1);
  }
  std::vector<query_eval::RowType> expected;
  for (size_t i = 0; i < data.second.size(); ++i) {
    query_eval::RowType row(schema.size());
    REQUIRE(data.second[i].size() == data.first.size());
    for (size_t j = 0; j < data.second[i].size(); ++j) {
      row[schemaMapping[data.first[j]]] = data.second[i][j];
    }
    expected.emplace_back(row);
  }
  std::sort(expected.begin(), expected.end());

  std::vector<query_eval::RowType> actual = result.data();
  std::sort(actual.begin(), actual.end());
  REQUIRE(actual == expected);
}

TEST_CASE("ResultTable Constructor") {
  using namespace query_eval;
  ResultTable t = std::vector<CellType> {3, 4, 5};
  REQUIRE(t.size() == 3);
}

TEST_CASE("ResultTable Schema"){
    using namespace query_eval;

    ResultTable t1(std::vector<CellType> {3, 4, 5});
    auto schema1 = t1.schema();
    REQUIRE(schema1.size() == 1);
    REQUIRE(schema1[0] == 0);

    ResultTable t2(std::vector<CellType> {1, 2, 3}, 2);
    auto schema2 = t2.schema();
    REQUIRE(schema2.size() == 1);
    REQUIRE(schema2[0] == 2);

    ResultTable t3(std::unordered_set<CellType> {1, 2, 3});
    auto schema3 = t3.schema();
    REQUIRE(schema3.size() == 1);
    REQUIRE(schema3[0] == 0);

    ResultTable t4(std::unordered_set<CellType> {1, 2, 3, 4}, 2);
    auto schema4 = t4.schema();
    REQUIRE(schema4.size() == 1);
    REQUIRE(schema4[0] == 2);

    ResultTable t5(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });
    auto schema5 = t5.schema();
    REQUIRE(schema5.size() == 3);
    REQUIRE(schema5[0] == 0);
    REQUIRE(schema5[1] == 1);
    REQUIRE(schema5[2] == 2);

}

TEST_CASE("ResultTable Reschema"){
    using namespace query_eval;

    ResultTable t1(std::vector<CellType> {3, 4, 5});
    t1 = t1.reschema({21});
    auto schema1 = t1.schema();
    REQUIRE(schema1.size() == 1);
    REQUIRE(schema1[0] == 21);

    ResultTable t2(std::vector<CellType> {1, 2, 3}, 2);
    t2 = t2.reschema({32});
    auto schema2 = t2.schema();
    REQUIRE(schema2.size() == 1);
    REQUIRE(schema2[0] == 32);

    ResultTable t3(std::unordered_set<CellType> {1, 2, 3});
    t3 = t3.reschema({0});
    auto schema3 = t3.schema();
    REQUIRE(schema3.size() == 1);
    REQUIRE(schema3[0] == 0);

    ResultTable t4(std::unordered_set<CellType> {1, 2, 3, 4}, 2);
    t4 = t4.reschema({3});
    auto schema4 = t4.schema();
    REQUIRE(schema4.size() == 1);
    REQUIRE(schema4[0] == 3);

    ResultTable t5(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });
    t5 = t5.reschema({7, 2, 103});
    auto schema5 = t5.schema();
    REQUIRE(schema5.size() == 3);
    REQUIRE(schema5[0] == 7);
    REQUIRE(schema5[1] == 2);
    REQUIRE(schema5[2] == 103);
}

TEST_CASE("ResultTable isSchemaExclusive"){
    using namespace query_eval;

    ResultTable t1(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });

    ResultTable t2(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });

    //t1 schema = {0,1,2}
    //t2 schema = {0,1,2}
    REQUIRE(t1.isSchemaExclusive(t2) == false);
    REQUIRE(t2.isSchemaExclusive(t1) == false);

    t1 = t1.reschema({7, 2, 103});
    //t1 schema = {7,2,103}
    //t2 schema = {0,1,2}
    REQUIRE(t1.isSchemaExclusive(t2) == false);
    REQUIRE(t2.isSchemaExclusive(t1) == false);

    t2 = t2.reschema({1, 3, 5});
    //t1 schema = {7,2,103}
    //t2 schema = {1,3,5}
    REQUIRE(t1.isSchemaExclusive(t2) == true);
    REQUIRE(t2.isSchemaExclusive(t1) == true);
}

TEST_CASE("ResultTable isSubschema"){
    using namespace query_eval;
    using namespace query_eval;

    ResultTable t1(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });

    ResultTable t2(std::vector<query_eval::RowType> {
            {4,2,3}, {1,2,2}, {2,3,4}
    });

    ResultTable t3(std::vector<query_eval::RowType> {
            {4,2}, {1,2}, {2,3}
    });

    //t1 schema = {0,1,2}
    //t2 schema = {0,1,2}
    //t3 schema = {0,1}
    REQUIRE(t1.isSubschema(t2) == true);
    REQUIRE(t2.isSubschema(t1) == true);
    REQUIRE(t3.isSubschema(t1) == false);
    REQUIRE(t1.isSubschema(t3) == true);
    REQUIRE(t1.isSubschema(t1) == true);

    t3 = t3.reschema({1,2});
    //t1 schema = {0,1,2}
    //t2 schema = {0,1,2}
    //t3 schema = {1,2}
    REQUIRE(t1.isSubschema(t3) == true);
    REQUIRE(t3.isSubschema(t1) == false);

    t2 = t2.reschema({0,1,3});
    //t1 schema = {0,1,2}
    //t2 schema = {0,1,3}
    //t3 schema = {1,2}
    REQUIRE(t1.isSubschema(t2) == false);
    REQUIRE(t2.isSubschema(t1) == false);

    t3 = t3.reschema({1,3});
    //t1 schema = {0,1,2}
    //t2 schema = {0,1,3}
    //t3 schema = {1,3}
    REQUIRE(t1.isSubschema(t3) == false);
    REQUIRE(t3.isSubschema(t1) == false);

}

TEST_CASE("ResultTable NaturalJoin") {
  using namespace query_eval;
  using TableData = std::pair<std::vector<SchemaType>, std::vector<RowType>>;

  std::vector<std::pair<std::vector<TableData>, TableData>> testcases = {
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
          {0}, //Schema
          {{1}, {2}, {3}, {4}, {5}, {6}}
        },
        {
          {0},
          {{4}, {5}, {6}, {7}, {8}, {9}}
        }
      },
      { // Result
        {0},
        {{4}, {5}, {6}}
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {0, 1}, // Schema
          { //Rows
            {1, 2},
            {2, 3},
            {3, 4},
            {4, 5}
          }
        },
        {
            {0, 1},
          {
            {1, 2},
            {3, 2},
            {3, 4},
            {5, 4}
          }
        }
      },
      { // Result
            {0, 1},
          {
            {1, 2},
            {3, 4}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {0, 1}, // Schema
          { // Rows
            {1, 2},
            {2, 3},
            {3, 4},
            {4, 5}
          }
        },
        {
            {1, 0},
          {
            {1, 2},
            {3, 2},
            {3, 4},
            {5, 4}
          }
        }
      },
      { // Result
            {0, 1},
          {
            {2, 3},
            {4, 5}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {0, 1}, // Schema
          { // Rows
            {1, 10},
            {2, 20},
            {3, 30},
            {4, 40},
            {5, 50},
          }
        },
        {
            {0, 3},
          {
            {1, 10},
            {1, 11},
            {3, 1},
            {4, 50},
            {15, 40},
            {20, 40}
          }
        }
      },
      { // Result
            {0, 1, 3},
          {
            {1, 10, 10},
            {1, 10, 11},
            {3, 30, 1},
            {4, 40, 50}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {0, 1}, // Schema
          { // Rows
            {4, 40},
            {5, 50},
          }
        },
        {
            {2, 3},
          {
            {1, 10},
            {3, 1},
            {4, 50},
            {20, 40}
          }
        }
      },
      { // Result
            {2, 3, 0, 1},
          {
            {1, 10, 4, 40},
            {3, 1, 4, 40},
            {4, 50, 4, 40},
            {20, 40, 4, 40},
            {1, 10, 5, 50},
            {3, 1, 5, 50},
            {4, 50, 5, 50},
            {20, 40, 5, 50}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {3, 1, 9}, // Schema
          { // Rows
            {40, 50, 1000},
            {50, 50, 2000},
            {40, 1, 3000}
          }
        },
        {
            {0, 1, 2, 3},
          {
            {1, 10, 4, 40},
            {3, 1, 4, 40},
            {4, 50, 4, 40},
            {20, 40, 4, 40},
            {1, 10, 5, 50},
            {3, 1, 5, 50},
            {4, 50, 5, 50},
            {20, 40, 5, 50}
          }
      }
      },
      { // Result
            {0, 1, 2, 3, 9},
          {
            {3, 1, 4, 40, 3000},
            {4, 50, 4, 40, 1000},
            {4, 50, 5, 50, 2000}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {3, 1, 9}, // Schema
          { // Rows
            {40, 50, 1000},
            {50, 50, 2000},
            {40, 1, 3000}
          }
        },
        { // TableData
            {2, 3}, // Schema
          { // Rows
            {4, 40},
            {5, 50},
          }
        },
        {
            {0, 1},
          {
            {1, 10},
            {3, 1},
            {4, 50},
            {20, 40}
          }
        }
      },
      { // Result
            {0, 1, 2, 3, 9},
          {
            {3, 1, 4, 40, 3000},
            {4, 50, 4, 40, 1000},
            {4, 50, 5, 50, 2000}
          }
      }
    },
    { // pair(tables to join, result)
      { //std::vector<TableData>
        { // TableData
            {9}, // Schema
          { // Rows

          }
        },
        {
            {0, 1, 2, 3},
          {
            {1, 10, 4, 40},
            {3, 1, 4, 40},
            {4, 50, 4, 40},
            {20, 40, 4, 40},
            {1, 10, 5, 50},
            {3, 1, 5, 50},
            {4, 50, 5, 50},
            {20, 40, 5, 50}
          }
      }
      },
      { // Result
            {0, 1, 2, 3, 9},
          {

          }
      }
    },
  };
  int cnt = 0;
  for (const auto&[tableList, expected]:testcases) {
    DYNAMIC_SECTION("ResultTable NaturalJoin " + std::to_string(++cnt)) {
      if (tableList.size() == 0) continue;
      ResultTable actual = generateResultTable(tableList[0]);
      for (size_t i = 1; i < tableList.size(); ++i) {
        actual = actual * generateResultTable(tableList[i]);
      }
      checkEqual(actual, expected);
    }
  }
}


TEST_CASE("ResultTable Project"){
    using namespace query_eval;

    TableData td1 = {
        {2, 3, 0, 1}, // Schema
        { // Rows
          {1, 10, 4, 40},
          {3, 1, 4, 40},
          {4, 50, 4, 40},
          {20, 40, 4, 40},
          {1, 10, 5, 50},
          {3, 1, 5, 50},
          {4, 50, 5, 50},
          {20, 40, 5, 50}
        }
    };

    std::vector<std::tuple<TableData, std::vector<SchemaType>, TableData>> testcases
    = {
      { // Two row projection
        td1,
        {  // To project
          0, 2
        },
        { // Result
          {2, 0}, // Schema
          { // Rows
            {1, 4},
            {3, 4},
            {4, 4},
            {20, 4},
            {1, 5},
            {3, 5},
            {4, 5},
            {20, 5}
          }
        }
      },
      { // Single Row
        td1,
        {  // To project
          1
        },
        { // Result
          {1}, // Schema
          { // Rows
            {40},
            {50}
          }
        }
      },
      { // Large Table
        {
          { // Schema
            10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
            40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
            50, 51, 52, 53, 54, 55, 56, 57, 58, 59
          },
          {
            { // First row
              10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
              20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
              30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
              40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
              50, 51, 52, 53, 54, 55, 56, 57, 58, 59
            },
            { // Second row
              0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
              10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
              20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
              30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
              40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
            }
          }
        },
        {  // To project
          21, 17, 20, 10, 59, 22, 28, 37, 42
        },
        { // Result
          {21, 17, 20, 10, 59, 22, 28, 37, 42}, // Schema
          { // Rows
            {21, 17, 20, 10, 59, 22, 28, 37, 42},
            {11, 7, 10, 0, 49, 12, 18, 27, 32}
          }
        }
      },
      { // Empty table
        {
          {5, 3, 2, 1},
          {}
        },
        {
          1, 2, 3
        },
        {
          {1, 2, 3},
          {

          }
        }
      }

    };
    int cnt = 0;
    for (const auto&[original, projection, expected]:testcases) {
      DYNAMIC_SECTION("ResultTable::project " + std::to_string(++cnt)) {
        ResultTable originalTable = generateResultTable(original);
        ResultTable actual = originalTable.project(projection);
        checkEqual(actual, expected);
      }
    }

}
