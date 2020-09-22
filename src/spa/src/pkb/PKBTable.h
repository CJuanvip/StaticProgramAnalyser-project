#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <RealtimeEvaluator.h>
namespace pkb {

template<class T, class U>
class PKBResult {
public:
  PKBResult(const T& left, const U& right) : left_(left), right_(right) { }
  const T& getLeft() const { return left_;}
  const U& getRight() const {return right_;}
private:
  T left_;
  U right_;
};

template<class T, class U>
class PKBTable {
public:
  PKBTable() : columnLeft_("__DEFAULT_UNNAMED_LEFT"), columnRight_("__DEFAULT_UNNAMED_RIGHT"), size_(0) {}
  PKBTable(const std::string& columnLeft, const std::string& columnRight)
  : columnLeft_(columnLeft), columnRight_(columnRight), size_(0) { }
  virtual const std::vector<PKBResult<T,U>>& select() const;
  virtual bool insert(const T& left, const U& right);
  virtual const std::unordered_set<U>& filterLeft(const T& left) const;
  virtual const std::unordered_set<T>& filterRight(const U& right) const;
  virtual const std::unordered_set<T>& selectLeft() const;
  virtual const std::unordered_set<U>& selectRight() const;
  virtual size_t count() const;
  virtual void dump(std::ostream& out = std::cout) const;
  virtual size_t countWithLeft(const T& left) const;
  virtual size_t countWithRight(const U& right) const;
  virtual bool contains(const T& left, const U& right) const;
  virtual void dumpSchema(std::ostream& out) const;
  virtual void clear();
private:
  std::unordered_set<T> emptyLeft_ = {};
  std::unordered_set<U> emptyRight_ = {};
  std::string columnLeft_;
  std::string columnRight_;
  size_t size_;
  std::unordered_set<T> leftKeys_;
  std::unordered_set<U> rightKeys_;
  std::unordered_map<T, std::unordered_set<U>> leftMap_;
  std::unordered_map<U, std::unordered_set<T>> rightMap_;
  std::vector<PKBResult<T,U>> results_;
};
// Not inherited as I do want to retain the constness of the PKBTable's method
template <class T, class U>
class RealtimePKBTable {
  public:
  RealtimePKBTable(RealtimeEvaluator<T,U>& evaluator, const std::string& columnLeft, const std::string& columnRight)
                  : table_(columnLeft, columnRight)
                  , evaluator_(evaluator) { }
  const std::vector<PKBResult<T,U>>& select();
  // no public insert
  const std::unordered_set<U>& filterLeft(const T& left);
  const std::unordered_set<T>& filterRight(const U& right);
  const std::unordered_set<T>& selectLeft();
  const std::unordered_set<U>& selectRight();
  size_t count();
  void dump(std::ostream& out = std::cout);
  size_t countWithLeft(const T& left);
  size_t countWithRight(const U& right);
  bool contains(const T& left, const U& right);
  void dumpSchema(std::ostream& out) const;
  void clear();
  private:
  PKBTable<T,U> table_;
  RealtimeEvaluator<T, U>& evaluator_;
  bool isInformationComplete_ = false; // TODO: We can have better tracking of partially filled states

  friend class PKBTable<T, U>;
};
}

// AOT PKB TABLE
namespace pkb {

template<class T, class U>
const std::vector<PKBResult<T, U>>& PKBTable<T,U>::select() const {
  return results_;
}

template<class T, class U>
bool PKBTable<T,U>::insert(const T& left, const U& right) {
  if (leftMap_.find(left) == leftMap_.end()) {
    leftMap_.emplace(left, std::unordered_set<U>{});
  }
  if (leftMap_[left].find(right) == leftMap_[left].end()) {
    leftMap_[left].emplace(right);
    leftKeys_.emplace(left);
    results_.emplace_back(left, right);
  } else {
    return false;
  }

  if (rightMap_.find(right) == rightMap_.end()) {
    rightMap_.emplace(right, std::unordered_set<T>{});
  }
  rightMap_[right].emplace(left);
  rightKeys_.emplace(right);
  ++size_;
  return true;
}

template<class T, class U>
const std::unordered_set<U>& PKBTable<T,U>::filterLeft(const T& left) const {
  if (leftMap_.find(left) != leftMap_.end()) {
    return leftMap_.at(left);
  } else {
    return emptyRight_;
  }
}

template<class T, class U>
const std::unordered_set<T>& PKBTable<T,U>::filterRight(const U& right) const {
  if (rightMap_.find(right) != rightMap_.end()) {
    return rightMap_.at(right);
  } else {
    return emptyLeft_;
  }
}

template<class T, class U>
const std::unordered_set<T>& PKBTable<T,U>::selectLeft() const {
  return leftKeys_;
}

template<class T, class U>
const std::unordered_set<U>& PKBTable<T,U>::selectRight() const {
  return rightKeys_;
}

template<class T, class U>
size_t PKBTable<T,U>::count() const {
  return size_;
}

template<class T, class U>
void PKBTable<T,U>::dump(std::ostream& out) const {
  auto results = select();
  for (auto& result : results) {
    out << result.getLeft() << ", " << result.getRight() << '\n';
  }
}

template<class T, class U>
size_t PKBTable<T,U>::countWithLeft(const T& left) const {
  if (leftMap_.find(left) == leftMap_.end()) {
    return 0;
  }
  else {
    return leftMap_.at(left).size();
  }
}

template<class T, class U>
size_t PKBTable<T,U>::countWithRight(const U& right) const {
  if (rightMap_.find(right) == rightMap_.end()) {
    return 0;
  }
  else {
    return rightMap_.at(right).size();
  }
}

template<class T, class U>
bool PKBTable<T,U>::contains(const T& left, const U& right) const {
  if (leftMap_.find(left) == leftMap_.end()) {
    return false;
  }
  return leftMap_.at(left).find(right) != leftMap_.at(left).end();
}

template<class T, class U>
void PKBTable<T,U>::dumpSchema(std::ostream& out) const {
  out << "Row left: " << this->columnLeft_ << "(" << typeid(T).name() << "); Row right: " << this->columnRight_ << "(" << typeid(U).name() << ").";
}

template<class T, class U>
void PKBTable<T,U>::clear() {
  leftKeys_.clear();
  rightKeys_.clear();
  leftMap_.clear();
  rightMap_.clear();
  results_.clear();
  size_ = 0;
}
}
// REAL TIME PKB TABLE
namespace pkb {
template<class T, class U>
const std::vector<PKBResult<T,U>>& RealtimePKBTable<T, U>::select() {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.select(table_);
  }
  return table_.select();
}

template<class T, class U>
const std::unordered_set<U>& RealtimePKBTable<T, U>::filterLeft(const T& left) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.filterLeft(left, table_);
  }
  return table_.filterLeft(left);
}

template<class T, class U>
const std::unordered_set<T>& RealtimePKBTable<T, U>::filterRight(const U& right) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.filterRight(right, table_);
  }
  return table_.filterRight(right);
}

template<class T, class U>
const std::unordered_set<T>& RealtimePKBTable<T, U>::selectLeft() {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.select(table_);
  }
  return table_.selectLeft();
}

template<class T, class U>
const std::unordered_set<U>& RealtimePKBTable<T, U>::selectRight() {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.select(table_);
  }
  return table_.selectRight();
}

template<class T, class U>
size_t RealtimePKBTable<T, U>::count() {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.select(table_);
  }
  return table_.count();
}

template<class T, class U>
void RealtimePKBTable<T, U>::dump(std::ostream& out) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.select(table_);
  }
  table_.dump(out);
}
template<class T, class U>
size_t RealtimePKBTable<T, U>::countWithLeft(const T& left) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.filterLeft(left, table_);
  }
  return table_.countWithLeft(left);
}
template<class T, class U>
size_t RealtimePKBTable<T, U>::countWithRight(const U& right) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.filterRight(right, table_);
  }
  return table_.countWithRight(right);
}
template<class T, class U>
bool RealtimePKBTable<T, U>::contains(const T& left, const U& right) {
  if (!isInformationComplete_) {
    clear();
    isInformationComplete_ = evaluator_.contains(left, right, table_);
  }
  return table_.contains(left, right);
}
template<class T, class U>
void RealtimePKBTable<T, U>::dumpSchema(std::ostream& out) const {
  table_.dumpSchema(out);
}

template<class T, class U>
void RealtimePKBTable<T, U>::clear() {
  table_.clear();
  isInformationComplete_ = false;
}

}
