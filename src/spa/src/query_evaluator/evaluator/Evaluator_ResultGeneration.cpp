#include "Evaluator.h"
#include "Visitor.h"
#include <vector>
#include <unordered_map>
#include <SPAAssert.h>

namespace query_eval {
  /**
   * =================================================================
   * Stage 5: Result Generation
   * =================================================================
   */
  void Evaluator::generateResultTables() {
    std::unordered_map<SchemaType, bool> resultComputed;
    for (auto &synonymIndex : resultList_) {
      resultComputed[synonymIndex] = false;
    }

    size_t numEntity = entityType_.size();
    UnionFindDisjointSet<SchemaType> ufds;
    for (const auto &table:clauseTables_) {
      const std::vector<SchemaType>& schema = table.schema();
      SPA_ASSERT(schema.size() > 0);
      if (table.size() == 0) {
        hasResult_ = false;
        return;
      }
      for (size_t i = 0; i < schema.size(); ++i) {
        if (resultComputed.count(schema[i]) == 1) {
          resultComputed[schema[i]] = true;
        }
        for (size_t j = i+1; j < schema.size(); ++j) {
          ufds.merge(schema[i], schema[j]);
        }
      }
    }

    for (auto&[synonymIndex, computed]: resultComputed) {
      if (!computed) {
         clauseTables_.emplace_back(
          cache_.select(entityType_[synonymIndex])
          .reschema({synonymIndex})
        );
      }
    }

    std::vector<std::vector<std::reference_wrapper<const ResultTable>>>
        tempTableGroups(numEntity);

    std::vector<std::pair<std::vector<std::reference_wrapper<const ResultTable>>, std::vector<SchemaType>>>
        joinTableGroups;

    for (const auto &table:clauseTables_) {
      const auto &schema = table.schema();
      SPA_ASSERT(ufds.parent(schema[0]) < numEntity);
      tempTableGroups[ufds.parent(schema[0])].emplace_back(table);
    }

    for (size_t index = 0; index < numEntity; ++index) {
      if (tempTableGroups[index].size() == 0) continue;
      std::unordered_set<SchemaType> requiredSchemaSet;
      for (const auto &table:tempTableGroups[index]) {
        const std::vector<SchemaType> &schema = table.get().schema();
        for (auto &x:schema) {
          if (resultComputed.count(x) > 0) {
            requiredSchemaSet.emplace(x);
          }
        }
      }
      if (requiredSchemaSet.size() == 0) {
        if (joinAndCheckResultExist(tempTableGroups[index]) == false) {
          hasResult_ = false;
          return;
        }
      } else {
        std::vector<SchemaType> requiredSchema;
        for (const auto &schema:requiredSchemaSet) {
          requiredSchema.emplace_back(schema);
        }

        joinTableGroups.emplace_back(tempTableGroups[index], requiredSchema);
      }
    }

    for (auto &[tableGroup, requiredSchema] : joinTableGroups) {
      groupResults_.emplace_back(joinTables(tableGroup, requiredSchema));
      if (groupResults_.back().size() == 0) {
        hasResult_ = false;
        return;
      }
    }
  }

  bool Evaluator::joinAndCheckResultExist(const std::vector<std::reference_wrapper<const ResultTable>> &tableList) const {
    // Will optimize in future using pipelined join algorithm
    SPA_ASSERT(tableList.size() > 0);
    return joinTables(tableList, {}).size() > 0;
  }
  ResultTable Evaluator::joinTables(
    std::vector<std::reference_wrapper<const ResultTable>> tableList,
    const std::vector<SchemaType> &requiredSchema
  ) const {
    SPA_ASSERT(tableList.size() > 0);
    if (tableList.size() == 1) {
      return tableList[0].get().project(requiredSchema);
    }
    // Maintain statistics of column count
    std::vector<size_t> columnCount(entityType_.size(), 0);
    for (const auto &column: requiredSchema) {
      // Instead of count++, just set to '1'
      columnCount[column] = 1;
    }
    for (const auto &table: tableList) {
      const auto &tableSchema = table.get().schema();
      for (const auto &column: tableSchema) {
        ++columnCount[column];
      }
    }
    // Stored here so can provide a reference...
    std::vector<ResultTable> projectedResultTables;
    // Need to reserve so that the references won't be invalidated
    projectedResultTables.reserve(tableList.size()*2);
    for (size_t i = 0; i < tableList.size(); ++i) {
      const auto &tableSchema = tableList[i].get().schema();
      std::vector<SchemaType> newSchema;
      newSchema.reserve(tableSchema.size());
      for (const auto &column: tableSchema) {
        if (columnCount[column] > 1) {
          newSchema.emplace_back(column);
        } else {
          columnCount[column] = 0;
        }
      }
      // Pre-emptive projection
      if (newSchema.size() != tableSchema.size()) {
        SPA_ASSERT(newSchema.size() > 0);
        projectedResultTables.emplace_back(tableList[i].get().project(newSchema));
        tableList[i] = projectedResultTables.back();
      }
    }
    while(tableList.size() > 1) {
      size_t x, y;
      bool found = false;
      for (size_t i = 0; i < tableList.size() && !found; ++i) {
        for (size_t j = 0; j < tableList.size() && !found; ++j) {
          if (i == j) continue;
          if (tableList[i].get().size() * tableList[j].get().schema().size() > tableList[j].get().size())
            continue;
          if (tableList[j].get().isSubschema(tableList[i].get())) {
            // x can be merged into y
            x = i;
            y = j;
            found = true;
          }
        }
      }
      if (!found) break;
      for (const auto& column: tableList[x].get().schema()) {
        --columnCount[column];
      }
      std::unordered_set<SchemaType> newSchema;
      for (const auto& column: tableList[y].get().schema()) {
        if (columnCount[column] == 1) {
          columnCount[column] = 0;
        } else {
          newSchema.emplace(column);
        }
      }
      const auto& tableRef = tableList.emplace_back(
        projectedResultTables.emplace_back(
          tableList[x].get().joinAndProject(tableList[y].get(), newSchema)
        )
      );
      if (x > y) std::swap(x, y);
      tableList.erase(tableList.begin() + y);
      tableList.erase(tableList.begin() + x);
      if (tableRef.get().size() == 0) break;
    }

    // Find minimum table size
    sort(tableList.begin(), tableList.end(),
      [&](const std::reference_wrapper<const ResultTable> a, const std::reference_wrapper<const ResultTable> b) {
        return a.get().size() < b.get().size();
      });

    std::vector<bool> joined(tableList.size(), false);
    ResultTable result = tableList[0].get();
    size_t joinCount = 1;
    joined[0] = true;
    while (result.size() > 0 && joinCount < tableList.size()) {
      // Find subschema joins if possible
      bool hasJoin = false;
      for (size_t i = 1; i < tableList.size(); ++i) {
        if (joined[i]) continue;
        const ResultTable &table = tableList[i].get();
        if (result.isSubschema(table)) {
          std::unordered_set<SchemaType> newSchema;
          for (const auto &column: result.schema()) {
            newSchema.emplace(column);
          }
          for (const auto &column: table.schema()) {
            if (newSchema.find(column) != newSchema.end()) {
              --columnCount[column];
              // Project it out if it only exists because of this join
              if (columnCount[column] == 1) {
                columnCount[column] = 0;
                newSchema.erase(column);
              }
            } else {
              newSchema.emplace(column);
            }
          }
          result = result.joinAndProject(table, newSchema);
          joined[i] = true;
          ++joinCount;
          hasJoin = true;
          break;
        }
      }
      if (hasJoin) continue;

      for (size_t i = 1; i < tableList.size(); ++i) {
        if (joined[i]) continue;
        const ResultTable &table = tableList[i].get();
        if (!result.isSchemaExclusive(table)) {

          std::unordered_set<SchemaType> newSchema;
          for (const auto &column: result.schema()) {
            newSchema.emplace(column);
          }
          for (const auto &column: table.schema()) {
            if (newSchema.find(column) != newSchema.end()) {
              --columnCount[column];
              // Project it out if it only exists because of this join
              if (columnCount[column] == 1) {
                columnCount[column] = 0;
                newSchema.erase(column);
              }
            } else {
              newSchema.emplace(column);
            }
          }
          result = result.joinAndProject(table, newSchema);
          joined[i] = true;
          ++joinCount;
          hasJoin = true;
          break;
        }
      }
      SPA_ASSERT(hasJoin);
    }
    return result;
  }

  std::list<std::string> Evaluator::generateFinalResults() const {
    std::list<std::string> result;
    if (resultList_.size() == 0) {
      return {"true"};
    } else if (resultList_.size() == 1) {
      SPA_ASSERT(groupResults_.size() == 1);
      SchemaType resultIndex = resultList_[0];
      ResultTable final = groupResults_.at(0).project({resultIndex});
      for (auto &row:final.data()) {
        result.emplace_back(cache_.getName(row[0]));
      }
      return result;
    } else {
      ResultTable final = groupResults_[0];
      for (size_t i = 1; i < groupResults_.size(); ++i) {
        final = final * groupResults_[i];
      }
      std::vector<SchemaType> schemaIndex;
      schemaIndex.reserve(resultList_.size());
      for (const auto& result:resultList_) {
        schemaIndex.emplace_back(final.getSchemaIndex(result).value());
      }
      for (const auto& row: final.data()) {
        std::string &str = result.emplace_back(cache_.getName(row[schemaIndex[0]]));
        for (size_t i = 1; i < schemaIndex.size(); ++i) {
          str += " ";
          str += cache_.getName(row[schemaIndex[i]]);
        }
      }
      return result;
    }
  }
};
