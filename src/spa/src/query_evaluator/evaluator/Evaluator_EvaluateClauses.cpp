#include "Evaluator.h"
#include "Visitor.h"

namespace query_eval {
  /**
   * =================================================================
   * Stage 4: Clause Evaluation
   * =================================================================
   */

  bool Evaluator::processBoolean() {
    for (const auto &with: booleanWiths_) {
      if (!processBooleanWith(with))
        return false;
    }

    for (const auto &suchThat: booleanSuchThats_) {
      if (!processBooleanSuchThat(suchThat))
        return false;
    }
    return true;
  }

  bool Evaluator::processOneSynonym() {
    for (const auto &with: oneSynonymWiths_) {
      if (!processOneSynonymWith(with))
        return false;
    }
    for (const auto &suchThat: oneSynonymSuchThats_) {
      if (!processOneSynonymSuchThat(suchThat))
        return false;
    }
    return true;
  }

  bool Evaluator::processTwoSynonym() {
    for (const auto &with: twoSynonymWiths_) {
      if (!processTwoSynonymWith(with))
        return false;
    }
    for (const auto &suchThat: twoSynonymSuchThats_) {
      if (!processTwoSynonymSuchThat(suchThat))
        return false;
    }
    return true;
  }

};
