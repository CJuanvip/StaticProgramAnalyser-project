#pragma once
#include <vector>

namespace parser {

std::vector<std::string> tokenizeSimple(const std::string& s);
std::vector<std::string> tokenizeQuery(const std::string& s);

}