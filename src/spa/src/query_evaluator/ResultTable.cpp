#include "ResultTable.h"
#include <SPAAssert.h>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace query_eval {
  ResultTable::ResultTable(const CellType &data, SchemaType colId) {
    schema_[colId] = 0;
    schemaRow_ = {colId};
    data_ = std::make_shared<std::vector<RowType>>();
    data_->emplace_back(1, data);
  }

  ResultTable::ResultTable(const std::vector<CellType>& data, SchemaType colId) {
    schema_[colId] = 0;
    schemaRow_ = {colId};
    data_ = std::make_shared<std::vector<RowType>>();
    data_->reserve(data.size());
    for (const CellType& cell : data) {
      data_->emplace_back(1, cell);
    }
  }

  ResultTable::ResultTable(const std::unordered_set<CellType>& data, SchemaType colId) {
    schema_[colId] = 0;
    schemaRow_ = {colId};
    data_ = std::make_shared<std::vector<RowType>>();
    data_->reserve(data.size());
    for (const CellType& cell : data) {
      data_->emplace_back(1, cell);
    }
  }

  ResultTable::ResultTable(const std::vector<pkb::PKBResult<CellType, CellType>> &data, SchemaType colId1, SchemaType colId2) {
    schema_[colId1] = 0;
    schema_[colId2] = 1;
    schemaRow_ = {colId1, colId2};
    data_ = std::make_shared<std::vector<RowType>>();
    data_->reserve(data.size());
    for (auto &pkbResult : data) {
      data_->emplace_back((RowType) {pkbResult.getLeft(), pkbResult.getRight()});
    }
  }

  ResultTable::ResultTable(const std::vector<RowType> &data) {
    data_ = std::make_shared<std::vector<RowType>>(data);
    if (data_->size() > 0) {
      size_t numCols = data[0].size();
      for (size_t i = 0; i < numCols; ++i) {
        schema_[i] = i;
        schemaRow_.emplace_back(i);
      }
    }
  }

  ResultTable ResultTable::operator* (const ResultTable &right) const {
    if (size() > right.size()) return right * (*this);
    SPA_ASSERT(size() <= right.size());
    const std::vector<SchemaType>& leftSchema = schema(), rightSchema = right.schema();
    // Compute intersection of schemas
    std::vector<SchemaType> intersection = leftSchema;
    intersection.resize(std::distance(intersection.begin(), std::remove_if(intersection.begin(), intersection.end(),
      [&](const SchemaType& x) {
        return right.schema_.count(x) == 0;
      })));

    if (intersection.size() == 0) {
      // Cross join, no intersection Found
      // Append 2 schemas
      std::vector<SchemaType> resultSchema = leftSchema;
      resultSchema.insert(resultSchema.end(), rightSchema.begin(), rightSchema.end());
      ResultTable result = ResultTable().reschema(resultSchema);
      result.data_->reserve(size() * right.size());
      for (const RowType &leftRow: data()) {
        for (const RowType &rightRow : right.data()) {
          // Append row of size 'resultSchema.size()'
          RowType &resultRow = result.data_->emplace_back(resultSchema.size());
          // std::copy can sometimes optimize to memmove
          std::copy(leftRow.begin(), leftRow.end(), resultRow.begin());
          std::copy(rightRow.begin(), rightRow.end(), resultRow.begin() + leftRow.size());
        }
      }
      return result;
    } else if (intersection.size() == 1) {
      std::unordered_map<CellType, std::vector<std::reference_wrapper<const RowType>>> leftSet(2*(right.size() + size()));
      size_t leftIntIndex = schema_.at(intersection[0]);
      size_t rightIntIndex = right.schema_.at(intersection[0]);

      // Generate result schema
      std::vector<size_t> rightOutIndex;
      std::vector<SchemaType> resultSchema = leftSchema;
      for (const auto &x : rightSchema) {
        if (x != intersection[0]) {
          resultSchema.emplace_back(x);
          rightOutIndex.emplace_back(right.schema_.at(x));
        }
      }

      for (const RowType &row: data()) {
        leftSet[row[leftIntIndex]].emplace_back(row);
      }

      ResultTable result = ResultTable().reschema(resultSchema);
      result.data_->reserve(size() + right.size());
      for (const RowType &rightRow: right.data()) {
        auto it = leftSet.find(rightRow[rightIntIndex]);
        if (it == leftSet.end()) continue;
        for (const auto &leftRow : it->second) {
          RowType &row = result.data_->emplace_back(resultSchema.size());
          std::copy(leftRow.get().begin(), leftRow.get().end(), row.begin());
          for (size_t i = 0; i < rightOutIndex.size(); ++i) {
            row[i + leftSchema.size()] = rightRow[rightOutIndex[i]];
          }
        }
      }
      // result.data_->shrink_to_fit();
      return result;
    }
    else if (intersection.size() == leftSchema.size()) {
      intersection = leftSchema;
      std::unordered_set<RowType> leftSet(2*(right.size() + size()));
      std::vector<size_t> rightIntIndex;
      for (const auto &x: intersection) {
        rightIntIndex.emplace_back(right.schema_.at(x));
      }

      for (const RowType &row: data()) {
        // Entire row is the key
        leftSet.emplace(row);
      }

      ResultTable result = ResultTable().reschema(rightSchema);
      result.data_->reserve(right.size());
      // RowType key(rightIntIndex.size());
      for (const RowType &rightRow: right.data()) {
        // Extract key
        // for (size_t i = 0; i < rightIntIndex.size(); ++i) {
        //   key[i] = rightRow[rightIntIndex[i]];
        // }
        RowType key;
        for (const auto &idx: rightIntIndex) {
          key.emplace_back(rightRow[idx]);
        }
        auto it = leftSet.find(key);
        if (it == leftSet.end()) continue;
        result.data_->emplace_back(rightRow);
      }
      // If memory concious
      // result.data_->shrink_to_fit();
      return result;
    }
    else if (intersection.size() == rightSchema.size()) {
      intersection = rightSchema;
      std::unordered_set<RowType> rightSet(2*(size() + right.size()));
      std::vector<size_t> leftIntIndex;
      for (const auto &x: intersection) {
        leftIntIndex.emplace_back(schema_.at(x));
      }
      for (const RowType &row: right.data()) {
        rightSet.emplace(row);
      }

      ResultTable result = ResultTable().reschema(leftSchema);
      result.data_->reserve(size());
      // RowType key(leftIntIndex.size());
      for (const RowType &leftRow: data()) {
        // Extract key
        RowType key;
        for (const auto &idx: leftIntIndex) {
          key.emplace_back(leftRow[idx]);
        }
        auto it = rightSet.find(key);
        if (it == rightSet.end()) continue;
        result.data_->emplace_back(leftRow);
      }
      // If memory concious
      // result.data_->shrink_to_fit();
      return result;
    }
    else {
      std::unordered_map<RowType, std::vector<std::reference_wrapper<const RowType>>> leftSet(2*(right.size() + size()));
      std::vector<size_t> leftIntIndex, rightIntIndex, rightOutIndex;
      std::unordered_set<size_t> intSet;
      for (const auto &x: intersection) {
        leftIntIndex.emplace_back(schema_.at(x));
        rightIntIndex.emplace_back(right.schema_.at(x));
        intSet.emplace(x);
      }

      // Generate result schema
      std::vector<SchemaType> resultSchema = leftSchema;
      for (const auto &x : rightSchema) {
        if (intSet.count(x) == 0) {
          resultSchema.emplace_back(x);
          rightOutIndex.emplace_back(right.schema_.at(x));
        }
      }
      for (const RowType &row: data()) {
        // Extract key
        RowType key;
        for (const auto &idx: leftIntIndex) {
          key.emplace_back(row[idx]);
        }
        leftSet[key].emplace_back(row);
      }

      ResultTable result = ResultTable().reschema(resultSchema);
      result.data_->reserve((size() + right.size()) * (leftSchema.size() + rightSchema.size() - intersection.size()));
      for (const RowType &rightRow: right.data()) {
        RowType key;
        for (const auto &idx: rightIntIndex) {
          key.emplace_back(rightRow[idx]);
        }
        auto it = leftSet.find(key);
        if (it == leftSet.end()) continue;
        for (const auto &leftRow : it->second) {
          RowType &row = result.data_->emplace_back(resultSchema.size());
          std::copy(leftRow.get().begin(), leftRow.get().end(), row.begin());
          for (size_t i = 0; i < rightOutIndex.size(); ++i) {
            row[i + leftSchema.size()] = rightRow[rightOutIndex[i]];
          }
        }
      }
      // If memory concious
      // result.data_->shrink_to_fit();
      return result;
    }
  }

  ResultTable ResultTable::joinAndProject(const ResultTable &right, const std::unordered_set<SchemaType> &schema) const {
    auto intersection = getIntersection(right);
    bool preserveIntersection = false;
    for (auto &column: intersection) {
      if (schema.find(column) != schema.end()) {
        preserveIntersection = true;
        break;
      }
    }
    if (preserveIntersection) {
      ResultTable result = (*this) * right;
      if (result.schema_.size() == schema.size()) {
        return result;
      } else {
        std::vector<SchemaType> resultSchema;
        resultSchema.reserve(schema.size());
        for (auto &column: schema) {
          resultSchema.emplace_back(column);
        }
        return result.project(resultSchema);
      }
    } else {
      std::vector<size_t> leftIntIdx, rightIntIdx;
      leftIntIdx.reserve(intersection.size());
      rightIntIdx.reserve(intersection.size());
      for (const auto& column: intersection) {
        leftIntIdx.emplace_back(schema_.at(column));
        rightIntIdx.emplace_back(right.schema_.at(column));
      }

      std::vector<SchemaType> leftSchema, rightSchema;
      std::vector<size_t> leftSchemaIdx, rightSchemaIdx;
      leftSchema.reserve(schema_.size() - intersection.size());
      leftSchemaIdx.reserve(schema_.size() - intersection.size());
      for (const auto& [col, index]: schema_) {
        if (schema.find(col) == schema.end()) continue;
        leftSchema.emplace_back(col);
        leftSchemaIdx.emplace_back(index);
      }

      rightSchema.reserve(right.schema_.size() - intersection.size());
      rightSchemaIdx.reserve(right.schema_.size() - intersection.size());
      for (const auto& [col, index]: right.schema_) {
        if (schema.find(col) == schema.end()) continue;
        rightSchema.emplace_back(col);
        rightSchemaIdx.emplace_back(index);
      }

      // TODO: Specialize for 1 intersection?
      std::unordered_map<RowType, std::vector<std::reference_wrapper<const RowType>>>
        leftSet(2*(right.size() + size())), rightSet(2*(right.size() + size()));


      for (const RowType &row: data()) {
        // Extract key
        RowType key;
        for (const auto &idx: leftIntIdx) {
          key.emplace_back(row[idx]);
        }
        leftSet[key].emplace_back(row);
      }

      for (const RowType &row: right.data()) {
        // Extract key
        RowType key;
        for (const auto &idx: rightIntIdx) {
          key.emplace_back(row[idx]);
        }
        auto it = leftSet.find(key);
        if (it == leftSet.end()) continue;
        rightSet[key].emplace_back(row);
      }
      // rightSet should be a subset of leftSet now
      SPA_ASSERT(rightSet.size() <= leftSet.size());

      std::vector<SchemaType> resultSchema = leftSchema;
      resultSchema.reserve(leftSchema.size() + rightSchema.size());
      for (const auto &column: rightSchema) {
        resultSchema.emplace_back(column);
      }
      ResultTable result = ResultTable().reschema(resultSchema);
      std::unordered_set<RowType> resultSet(2*(size() + right.size()));
      for (const auto &[key, leftList]: leftSet) {
        auto it = rightSet.find(key);
        if (it == rightSet.end()) continue;
        const auto& rightList = it->second;

        std::unordered_set<RowType> leftValSet(leftSet.size()), rightValSet(rightSet.size());
        // Extract remaining from left
        for (const auto &rowRef: leftList) {
          const auto &row = rowRef.get();
          RowType value;
          for (const auto &idx: leftSchemaIdx) {
            value.emplace_back(row[idx]);
          }
          leftValSet.emplace(std::move(value));
        }

        // Extract remaining from right
        for (const auto &rowRef: rightList) {
          const auto &row = rowRef.get();
          RowType value;
          for (const auto &idx: rightSchemaIdx) {
            value.emplace_back(row[idx]);
          }
          rightValSet.emplace(std::move(value));
        }

        if (leftSchema.size() == 0) {
          resultSet.merge(rightValSet);
        } else if (rightSchema.size() == 0) {
          resultSet.merge(leftValSet);
        } else {
          RowType row(resultSchema.size());
          for (const auto &leftRow: leftValSet) {
            std::copy(leftRow.begin(), leftRow.end(), row.begin());
            for (const auto &rightRow: rightValSet) {
              std::copy(rightRow.begin(), rightRow.end(), row.begin() + leftSchema.size());
              resultSet.emplace(row);
            }
          }
        }
      }
      result.data_->reserve(resultSet.size());
      for (auto it = resultSet.begin(); it != resultSet.end(); ) {
        result.data_->emplace_back(std::move(resultSet.extract(it++).value()));
      }
      return result;
    }
  }
  const std::vector<SchemaType>& ResultTable::schema() const {
    return schemaRow_;
  }

  std::optional<SchemaType> ResultTable::getSchemaIndex(const SchemaType &col) const {
    if (schema_.count(col) == 0) {
      return {};
    } else {
      return schema_.at(col);
    }
  }

  ResultTable ResultTable::reschema(const std::vector<SchemaType> &schema) const {
    ResultTable result = *this;
    result.schema_.clear();
    for (size_t i = 0; i < schema.size(); ++i) {
      result.schema_[schema[i]] = i;
    }
    result.schemaRow_ = schema;
    // Assert that all columns are unique
    SPA_ASSERT(result.schema_.size() == schema.size());
    return result;
  }

  bool ResultTable::isSchemaExclusive(const ResultTable &right) const {
    for (auto &col:right.schemaRow_) {
      if (schema_.find(col) != schema_.end()) return false;
    }
    return true;
  }

  bool ResultTable::isSubschema(const ResultTable &right) const {
    for (auto &col:right.schemaRow_) {
      if (schema_.find(col) == schema_.end()) return false;
    }
    return true;
  }

  std::vector<SchemaType> ResultTable::getIntersection(const ResultTable &right) const {
    std::vector<SchemaType> result;
    for (auto &col:right.schemaRow_) {
      if (schema_.find(col) != schema_.end()) {
        result.emplace_back(col);
      }
    }
    return result;
  }


  ResultTable ResultTable::filter (CellType value, SchemaType col) const {
    return ResultTable(value, col) * (*this);
  }

  ResultTable ResultTable::project (const std::vector<SchemaType> &schema) const {
    // Ensures all attributes of the new schema are in the current schema
    #if !defined (NO_ASSERT)
    for (auto col:schema) {
      SPA_ASSERT(schema_.find(col) != schema_.end());
    }
    #endif
    
    if (schema.size() == schema_.size()) {
      return *this;
    }
    if (schema.size() == 1) {
      std::unordered_set<CellType> projected(data_->size());
      SchemaType col = schema[0];
      size_t index = schema_.at(col);
      for (const auto &row : data()) {
        projected.emplace(row[index]);
      }
      return ResultTable(projected, col);
    } else {
      ResultTable result = ResultTable().reschema(schema);
      std::unordered_set<RowType> projected(2*(data_->size()));
      std::vector<size_t> colIdx;
      for (const auto &col:schema) {
        colIdx.emplace_back(schema_.at(col));
      }
      for (const auto &row : data()) {
        RowType projectedRow;
        for (const auto &idx: colIdx) {
          projectedRow.emplace_back(row[idx]);
        }
        projected.emplace(std::move(projectedRow));
      }
      result.data_->reserve(projected.size());
      for (auto it = projected.begin(); it != projected.end(); ) {
        result.data_->emplace_back(std::move(projected.extract(it++).value()));
      }
      return result;
    }
  }

  size_t ResultTable::size() const {
    return data_->size();
  }

}
