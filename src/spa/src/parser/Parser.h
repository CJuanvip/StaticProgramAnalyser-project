#pragma once
#include <string>
#include "AST.h"
#include "Combinator.h"

namespace parser {

std::string stripComments(const std::string& src);
ParseResult<common::Program> parseSimple(const std::string& src);
ParseResult<common::Query> parseQuery(const std::string& src);

}
