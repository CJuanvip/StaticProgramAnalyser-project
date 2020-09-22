#pragma once
#include <unordered_map>

namespace query_eval {
  template<typename T>
  class UnionFindDisjointSet {
  public:
    bool merge(const T &a, const T &b) {
      T pa = parent(a);
      T pb = parent(b);
      if (pa == pb) return false;
      parent_[pb] = pa;
      return true;
    }

    T parent(const T &x) {
      if (parent_.find(x) != parent_.end()) {
        // Path compression
        if (parent_[x] != x) {
          parent_[x] = parent(parent_[x]);
        }
        return parent_[x];
      } else {
        return x;
      }
    }
  private:
  std::unordered_map<T, T> parent_;

  };
}
