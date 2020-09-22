#pragma once

#include <unordered_map>
#include <vector>

namespace common {
  namespace mapper {

    // Helper function to inverse 1-to-1 mappings
    // unordered_map<T, U> -> unordered_map<U, T>
    template<typename T, typename U>
    const auto inverseOnetoOneMapper = [](const std::unordered_map<T, U> &m) -> std::unordered_map<U, T> {
      std::unordered_map<U, T> ret;
      for (const auto&[t, u]: m) ret[u] = t;
      return ret;
    };

    // Helper function to inverse many-to-1 mappings
    // unordered_map<T, U> -> unordered_map<U, vector<T>>
    template<typename T, typename U>
    const auto inverseManytoOneMapper = [](const std::unordered_map<T, U> &m) -> std::unordered_map<U, std::vector<T>> {
      std::unordered_map<U, std::vector<T>> ret;
      for (const auto&[t, u]: m) ret[u].emplace_back(t);
      return ret;
    };
  }
}
