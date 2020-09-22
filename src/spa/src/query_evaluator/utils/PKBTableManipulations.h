#pragma once
#include "pkb/PKBTable.h"
#include <unordered_set>
#include <SPAAssert.h>

namespace query_eval {
  // Temp fix!
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBRight(const pkb::PKBTable<T, U> &base, const std::unordered_set<U> &right) {
    pkb::PKBTable<T, U> result; //TO FIX
    for (const auto &r : right) {
      for (const auto &l : base.filterRight(r)) {
        result.insert(l, r);
      }
    }
    return result;
  }

  // Temp fix!
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBLeft(const pkb::PKBTable<T, U> &base, const std::unordered_set<T> &left) {
    pkb::PKBTable<T, U> result; //TO FIX
    for (const auto &l : left) {
      for (const auto &r : base.filterLeft(l)) {
        result.insert(l, r);
      }
    }
    return result;
  }

  // Temp fix!
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBLeftRight(const pkb::PKBTable<T, U> &base, const std::unordered_set<T> &left, const std::unordered_set<U> &right) {
    pkb::PKBTable<T, U> result; //TO FIX
    if (left.size() < right.size()) {
      for (const auto &l : left) {
        for (const auto &r : base.filterLeft(l)) {
          if (right.count(r) == 1)
            result.insert(l, r);
        }
      }
    } else {
      for (const auto &r : right) {
        for (const auto &l : base.filterRight(r)) {
          if (left.count(l) == 1)
            result.insert(l, r);
        }
      }
    }
    return result;
  }

  // Temp fix
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBRight(pkb::RealtimePKBTable<T, U> &base, const std::unordered_set<U> &right) {
    pkb::PKBTable<T, U> result; //TO FIX
    for (const auto &r: right) {
      for (const auto &l: base.filterRight(r)) {
        result.insert(l, r);
      }
    }
    return result;
  }

  // Temp fix
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBLeft(pkb::RealtimePKBTable<T, U> &base, const std::unordered_set<T> &left) {
    pkb::PKBTable<T, U> result; //TO FIX
    for (const auto &l: left) {
      for (const auto &r: base.filterLeft(l)) {
        result.insert(l, r);
      }
    }
    return result;
  }

  // Temp fix
  template<class T, class U>
  pkb::PKBTable<T, U> filterPKBLeftRight(pkb::RealtimePKBTable<T, U> &base, const std::unordered_set<T> &left, const std::unordered_set<U> &right) {
    pkb::PKBTable<T, U> result; //TO FIX
    for (const auto &l: left) {
      for (const auto &r: base.filterLeft(l)) {
        if (right.count(r) == 1)
          result.insert(l, r);
      }
    }
    return result;
  }

  // Temp fix!
  template<class T, class U>
  pkb::PKBTable<T, U> unionPKB(const pkb::PKBTable<T, U> &front, const pkb::PKBTable<T, U> &back) {
    pkb::PKBTable<T, U> result = front; // Copy
    for (const auto &row : back.select()) {
      result.insert(row.getLeft(), row.getRight());
    }
    SPA_ASSERT(result.count() >= std::max(back.count(), front.count()));
    return result;
  }
}
