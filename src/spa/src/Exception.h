#pragma once
const char* const ERROR_MESSAGE_NON_EXISTENT_PROC_NAME = "Call to non-existing procedure name!";
const char* const ERROR_MESSAGE_DUPLICATE_PROC_NAME = "Duplicate Procedure Name.";
const char* const ERROR_MESSAGE_CYCLIC_CALL = "Cyclic call in program detected!";
#include <exception>
class SemanticError: public std::exception {
public:
  explicit SemanticError(const char* message)
  : msg_(message) { }
  virtual ~SemanticError() throw() {}
  virtual const char* what() const throw() {
    return msg_.c_str();
  }
private:
  std::string msg_;
};