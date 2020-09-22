#include "TestWrapper.h"
#include "ConstructAST.h"
#include "parser/Parser.h"
#include <sstream>
#include <iostream>
#include <fstream>

// implementation code of WrapperFactory - do NOT modify the next 5 lines
AbstractWrapper* WrapperFactory::wrapper = 0;
AbstractWrapper* WrapperFactory::createWrapper() {
  if (wrapper == 0) wrapper = new TestWrapper;
  return wrapper;
}
// Do not modify the following line
volatile bool AbstractWrapper::GlobalStop = false;

// a default constructor
TestWrapper::TestWrapper(): cache_(mgr_) {

  // create any objects here as instance variables of this class
  // as well as any initialization required for your spa program
}

// method for parsing the SIMPLE source
void TestWrapper::parse(std::string filename) {
  std::fstream simpleFile;
  simpleFile.open(filename);
  if (!simpleFile.is_open()) {
    throw std::runtime_error("SIMPLE source cannot be opened.");
  }
#ifdef ANSWER_GEN
  name_ = filename.substr(0, filename.length()-11);
  std::ofstream answerFile;
  answerFile.open(name_ + "Answer.out");
  answerFile.flush();
  answerFile.close();
#endif
  // Reads the entire file into sourceCode
  std::stringstream strStream;
  strStream << simpleFile.rdbuf();
  std::string sourceCode = strStream.str();
  auto result = parser::parseSimple(sourceCode);
  if (!parser::success(result)) {
    std::cerr << "expected: " << parser::ParseFailure(result).expected << std::endl;
    std::cerr << "got: " << parser::ParseFailure(result).got << std::endl;
    return;
  }
  common::Program prog = parser::ParseSuccess(result).value;
  designExtractor_ = std::unique_ptr<ast::DesignExtractor>(new ast::DesignExtractor(mgr_));
  designExtractor_->initialize(prog);
  try {
    designExtractor_->extract();
  } catch (SemanticError& err) {
    std::terminate();
  }
  cache_ = query_eval::TableCacher(mgr_);
}

// method to evaluating a query
void TestWrapper::evaluate(std::string query, std::list<std::string>& results){
  auto result = parser::parseQuery(query);
#ifdef ANSWER_GEN
  std::ofstream simpleFile;
  simpleFile.open(name_ + "Answer.out", ios::app);
  simpleFile << query + "\n";
#endif
  if (!parser::success(result)) {
#ifdef ANSWER_GEN
    simpleFile << "none\n5000\n";
    simpleFile.flush();
    simpleFile.close();
#endif
    std::cerr << "expected: " << parser::ParseFailure(result).expected << std::endl;
    std::cerr << "got: " << parser::ParseFailure(result).got << std::endl;
    std::cerr << "query: " << query << std::endl;
    return;
  }
  query_eval::Evaluator evaluator(cache_);
  results = evaluator.evaluate(parser::ParseSuccess(result).value);
#ifdef ANSWER_GEN
  if (results.size()) {
    for (const auto &str: results) {
      simpleFile << str + ", ";
    }
  } else {
    simpleFile << "none\n";
  }
  simpleFile << "\n";
  simpleFile << "5000\n";
  simpleFile.flush();
  simpleFile.close();
#endif
  mgr_.clearCache();
  cache_.resetRealtimeRelationTables();
}
