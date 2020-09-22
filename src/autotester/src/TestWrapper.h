#ifndef TESTWRAPPER_H
#define TESTWRAPPER_H

#include <string>
#include <iostream>
#include <list>
#include <memory>
#include <pkb/PKBTableManager.h>
#include <query_evaluator/TableCacher.h>
#include <query_evaluator/Evaluator.h>
#include <DesignExtractor.h>
// include your other headers here
#include "AbstractWrapper.h"

class TestWrapper : public AbstractWrapper {
 public:
  // default constructor
  TestWrapper();

  // destructor
  ~TestWrapper();

  // method for parsing the SIMPLE source
  virtual void parse(std::string filename);

  // method for evaluating a query
  virtual void evaluate(std::string query, std::list<std::string>& results);

 private:
  std::unique_ptr<ast::DesignExtractor> designExtractor_;
  pkb::PKBTableManager mgr_;
  query_eval::TableCacher cache_;
#ifdef ANSWER_GEN
  std::string name_;
#endif
};

#endif
