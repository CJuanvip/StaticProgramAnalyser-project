#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <memory>
#include "pkb/PKBTable.h"

namespace query_eval
{
  using CellType = unsigned;
  using SchemaType = unsigned;
  using RowType = std::vector<CellType>;
  class ResultTable {
  public:
    ResultTable() {
      data_ = std::make_shared<std::vector<RowType>>();
    };
    ~ResultTable() {};

    // Constructor that takes in a single value
    ResultTable(const CellType &data, SchemaType col=0);
    // Constructor that takes in a container with 'CellType'
    ResultTable(const std::vector<CellType> &data, SchemaType colId=0);
    // Constructor that takes in data outputted from PKBTable's filterLeft/filterRight methods
    ResultTable(const std::unordered_set<CellType> &data, SchemaType colId=0);
    // Constructor that takes in data outputted from PKBTable's select method
    ResultTable(const std::vector<pkb::PKBResult<CellType, CellType>> &data, SchemaType colId1=0, SchemaType colId2=1);
    // Constructor that takes in data in a row form
    ResultTable(const std::vector<RowType> &data);


    // Get schema of the table
    const std::vector<SchemaType>& schema() const;
    // Change the schema of the table
    ResultTable reschema(const std::vector<SchemaType> &schema) const;
    // Check if the two table share any columns in common
    bool isSchemaExclusive(const ResultTable &right) const;
    // Check if table right's schema is a subset of the current table schema
    bool isSubschema(const ResultTable &right) const;
    // Returns the columns that are common to both tables
    std::vector<SchemaType> getIntersection(const ResultTable &right) const;
    // Returns the index of a particular column as stored in this table
    std::optional<SchemaType> getSchemaIndex(const SchemaType &col) const;

    // Operators
    ResultTable operator* (const ResultTable &right) const;
    // Specialization of the natural join operator that allows one to perform a projection on the result of the join
    ResultTable joinAndProject (const ResultTable &right, const std::unordered_set<SchemaType> &schema) const;
    // Filter the table based on a single value of one column
    ResultTable filter (CellType value, SchemaType col=0) const;
    // Project the table based on a new schema
    ResultTable project (const std::vector<SchemaType> &schema) const;

    // Returns the number of rows in the table
    size_t size() const;

    const std::vector<RowType>& data() const { return *data_; }

  private:
    std::shared_ptr<std::vector<RowType>> data_;
    std::unordered_map<SchemaType, size_t> schema_;
    std::vector<SchemaType> schemaRow_;
  };
}


namespace std {
  template<>
    class hash<query_eval::RowType> {
      public:
        size_t operator() (const query_eval::RowType &row) const {
          if (row.size() == 1) return row[0];
          size_t seed = 0;
          for (const auto& cell: row) {
            seed ^= cell + 0x9e3779b9 + (seed << 6) + (seed >> 2);
          }
          return seed;
        }
    };
};
