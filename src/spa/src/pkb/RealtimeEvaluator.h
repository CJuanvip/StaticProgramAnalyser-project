#pragma once

namespace pkb {
  template<class T, class U>
  class PKBTable;
  template <class T, class U>
  class RealtimeEvaluator {
  public:
    // Allow for fine-grained control;
    // precondition: table is empty
    // return true, if all information for that table is populated
    // or false otherwise
    virtual bool select(PKBTable<T, U>&) = 0;
    virtual bool filterLeft(const T& left, PKBTable<T, U>&) = 0;
    virtual bool filterRight(const U& right, PKBTable<T, U>&) = 0;
    virtual bool contains(const T& left, const U& right, PKBTable<T, U>&) = 0;
  };
}