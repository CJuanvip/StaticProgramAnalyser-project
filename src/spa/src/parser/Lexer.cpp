#include <string>
#include <regex>
#include "Lexer.h"

namespace parser {

const std::string simpleRegex = R"((?:\s*)([a-zA-Z]\w*|\d+|==|!=|<=|>=|\|\||&&|\S))";
const std::string pqlRegex = R"((?:\s*)(_".*?"_|".*?"|stmt#|Calls\*|Follows\*|Parent\*|Next\*|Affects\*|NextBip\*|AffectsBip\*|NextBip|AffectsBip|[a-zA-Z]\w*|\d+|==|!=|<=|>=|\|\||&&|\S))";

std::vector<std::string> tokenize(std::string s, const std::string& regex_string) {
  std::regex regex(regex_string);
  std::vector<std::string> tokens;
  std::smatch match;

  std::sregex_iterator iter(s.begin(), s.end(), regex);
  std::sregex_iterator end;
  for (; iter != end; iter++) {
    auto matched = iter->str(1);
    tokens.push_back(matched);
  }
  return tokens;
}

std::vector<std::string> tokenizeSimple(const std::string& s) {
  return tokenize(s, simpleRegex);
}

std::vector<std::string> tokenizeQuery(const std::string& s) {
  auto tokens = tokenize(s, pqlRegex);
  for (auto it = tokens.begin(); it != tokens.end();) {
    if (it->size() >= 2 && it->front() == '"' && it->back() == '"') {
      auto newTokens = tokenizeSimple(it->substr(1, it->size()-2));
      it = tokens.erase(it);
      it = tokens.insert(it, "\""); ++it;
      it = tokens.insert(it, newTokens.begin(), newTokens.end()); it += newTokens.size();
      it = tokens.insert(it, "\""); ++it;
    } else if (it->size() >= 4 && it->front() == '_' && it->back() == '_') {
      auto newTokens = tokenizeSimple(it->substr(2, it->size()-4));
      it = tokens.erase(it);
      it = tokens.insert(it, "_\""); ++it;
      it = tokens.insert(it, newTokens.begin(), newTokens.end()); it += newTokens.size();
      it = tokens.insert(it, "\"_"); ++it;
    } else {
      ++it;
    }
  }
  return tokens;
}

}
