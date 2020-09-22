#include "Evaluator.h"
#include "Visitor.h"
#include <unordered_map>
#include <SPAAssert.h>

namespace query_eval {


  std::list<std::string> Evaluator::evaluate(const Query &query) {
    std::list<std::string> result = {};
    if (query.isBooleanResult) {
      result.emplace_back("false");
      SPA_ASSERT(query.result.size() == 0);
    }

    // Stage 1: PQL Validation
    if (hasSemanticError(query)) return result;

    // Stage 2: Synonym Normalization
    if (!normalizeDeclarations(query.declarations)) return result;
    if (!normalizeResultList(query.result)) return result;
    if (!normalizeWithAttributes(query.withs)) return result;

    // Stage 3: Early Termination
    if (canEarlyTerminate(query)) return result;

    // Stage 4: Clause Evaluation
    sortSuchThats(query.suchThats);
    sortWiths(query.withs);


    if (!processBoolean()) return result;
    if (!processOneSynonym()) return result;
    hasResult_ = true;
    sortPatterns(query.patterns);
    if (!hasResult_) return result;

    if (!processTwoSynonym()) return result;


    // Stage 5: Result Generation
    generateResultTables();
    if (!hasResult_) return result;
    return generateFinalResults();
  }

  std::optional<SchemaType> Evaluator::getSynonymIndex(const AttrRef &synonym) const {
    auto it = synonymToIndex_.find(synonym);
    if (it == synonymToIndex_.end()) return {};
    else return it->second;
  }

  std::optional<SchemaType> Evaluator::getSynonymIndex(const Synonym &synonym) const {
    return getSynonymIndex((AttrRef) synonym);
  }

  // Helper function to indicate that there is only one result for this entity
  bool Evaluator::predetermineSynonym(const SchemaType &synonymIndex, const CellType &value) {
    auto it = synonymPredetermined_.find(synonymIndex);
    if (it == synonymPredetermined_.end()) {
      synonymPredetermined_[synonymIndex] = value;
      return true;
    } else {
      return it->second == value;
    }
  }

}
