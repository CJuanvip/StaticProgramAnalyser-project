#pragma once
#include <string>
#include <regex>
#include <optional>
#include <tuple>
#include <variant>
#include <type_traits>
#include <SPAAssert.h>

namespace parser {

template<typename T>
struct ParseSuccess;
struct ParseFailure;

template<typename T>
using ParseResult = std::variant<ParseSuccess<T>, ParseFailure>;

template<typename T>
bool success(ParseResult<T> result) {
  return result.index() == 0;
}

template<typename T>
struct ParseSuccess {
  T value;
  std::vector<std::string> remaining;
  ParseSuccess(T value, std::vector<std::string> remaining) : value(value), remaining(remaining) {}
  explicit ParseSuccess(ParseResult<T> result) : value(std::get<0>(result).value), remaining(std::get<0>(result).remaining) {}
};

struct ParseFailure {
  std::string expected;
  std::string got;
  ParseFailure(std::string expected, std::string got) : expected(expected), got(got) {}
  template<typename T>
  explicit ParseFailure(ParseResult<T> result) : expected(std::get<1>(result).expected), got(std::get<1>(result).got) {}
};

template<typename T>
ParseResult<T> fullyMatched(const ParseResult<T>& parse_result) {
  if (success(parse_result) && !ParseSuccess<T>(parse_result).remaining.empty()) {
    return ParseFailure("End of line", ParseSuccess<T>(parse_result).remaining[0]);
  }
  return parse_result;
}

// It is necessary to repeat the ResultType declaration in every derived class because type aliases
// cannot be inherited within templates (See: https://stackoverflow.com/q/39334150)

template<typename T>
struct Parser {
  using ValueType = T;
  using ResultType = ParseResult<T>;
  virtual ResultType parse(const std::vector<std::string>& s) const = 0;
};

// StringParser(s): Parser that consumes token s

struct StringParser : Parser<std::string> {
  using ValueType = std::string;
  using ResultType = typename Parser<std::string>::ResultType;
  std::string target;
  explicit StringParser(std::string s) : target(s) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    if (s.empty()) {
      return ParseFailure(target, "empty string");
    }
    if (s[0] == target) {
      return ParseSuccess(target, std::vector<std::string>(s.begin()+1, s.end()));
    }
    return ParseFailure(target, s[0]);
  }
};

// StringParser(r): Parser that consumes token that matches regex r exactly

struct RegexParser : Parser<std::string> {
  using ValueType = std::string;
  using ResultType = typename Parser<std::string>::ResultType;
  std::regex re;
  explicit RegexParser(std::string s) : re(s) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    if (s.empty()) {
      return ParseFailure("regex match", "empty string");
    }
    std::smatch match;
    std::regex_match(s[0], match, re);
    if (match.empty()) {
      // TODO: more meaningful error
      return ParseFailure("regex match", s[0]);
    }
    return ParseSuccess(std::string(match[0]), std::vector<std::string>(s.begin()+1, s.end()));
  }
};

// LazyParser enables recursion by separating the declaration of a parser from its definition.
// Unlike other parsers, ValueType is a template argument that has to be explicitly given when
// instantiating a LazyParser template since it cannot be deduced without constructor arguments.

template<typename ValueType>
struct LazyParserInstance : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  std::shared_ptr<std::unique_ptr<Parser<ValueType>>> ptr;
  template<typename ParserType>
  explicit LazyParserInstance(std::shared_ptr<std::unique_ptr<ParserType>> ptr) : ptr(ptr) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    SPA_ASSERT(*ptr != nullptr);
    return (*ptr)->parse(s);
  }
};

// Usage:
// - Default initialisation acts like "forward declaration"
// - LazyParser::get acts like a reference
// - LazyParser::define supplies the actual definition

template<typename ValueType>
struct LazyParser : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  std::shared_ptr<std::unique_ptr<Parser<ValueType>>> ptr = std::make_shared<std::unique_ptr<Parser<ValueType>>>();
  LazyParser() {};
  template<typename ParserType>
  void define(const ParserType& p) {
    ptr->reset(new ParserType(p));
  }
  LazyParserInstance<ValueType> get() const {
    return LazyParserInstance<ValueType>(ptr);
  }
  ResultType parse(const std::vector<std::string>& s) const override {
    SPA_ASSERT(*ptr != nullptr);
    return (*ptr)->parse(s);
  }
};

// Alt(p, ...): Runs a fixed sequence of parsers and returns result of the first successful parser

template<class T, class...>
struct first { using type = T; };

template<typename... ParserTypes>
struct Alt : Parser<typename first<ParserTypes...>::type::ValueType> {
  using ValueType = typename first<ParserTypes...>::type::ValueType;
  using ResultType = typename Parser<ValueType>::ResultType;
  static_assert(std::conjunction<std::is_same<ValueType, typename ParserTypes::ValueType>...>::value,
      "Parser types for Alt must be uniform");
  std::tuple<ParserTypes...> parsers;
  explicit Alt(ParserTypes... parsers) : parsers(parsers...) {}
  template<size_t I>
  ResultType parseImpl(const std::vector<std::string>& s) const {
    auto result = std::get<I>(parsers).parse(s);
    if (success(result)) {
      return ParseSuccess(result);
    }
    if constexpr (I+1 < sizeof...(ParserTypes)) {
      return parseImpl<I+1>(s);
    } else {
      return ParseFailure(result);
    }
  }
  ResultType parse(const std::vector<std::string>& s) const override {
    return parseImpl<0>(s);
  }
};

// Then(p, ...): Runs a fixed sequence of parsers and returns their results in a tuple

// Needed to trigger parameter pack expansion
template<typename ParserType>
using valueOf = typename ParserType::ValueType;

template<typename... ParserTypes>
struct Then : Parser<std::tuple<valueOf<ParserTypes>...>> {
  using ValueType = std::tuple<valueOf<ParserTypes>...>;
  using ResultType = typename Parser<ValueType>::ResultType;
  std::tuple<ParserTypes...> parsers;
  explicit Then(ParserTypes... parsers) : parsers(parsers...) {}
  template<size_t I>
  ResultType parseImpl(std::tuple<valueOf<ParserTypes>...> results, const std::vector<std::string>& s) const {
    if constexpr (I == sizeof...(ParserTypes)) {
      return ParseSuccess(results, s);
    } else {
      auto result = std::get<I>(parsers).parse(s);
      if (!success(result)) {
        return ParseFailure(result);
      }
      auto [value, tokens] = ParseSuccess(result);
      std::get<I>(results) = value;
      return parseImpl<I+1>(results, tokens);
    }
  }
  ResultType parse(const std::vector<std::string>& s) const override {
    return parseImpl<0>({}, s);
  }
};

// Some combinators define ValueType as a template argument with a default value as a trick to
// define a type alias to be used in specifying the base class Parser<ValueType>

// Many(p): Runs p multiple times, returns results in a vector

template<typename ParserType,
         typename ElementType = typename ParserType::ValueType,
         typename ValueType = typename std::vector<ElementType>>
struct Many : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType p;
  explicit Many(ParserType p) : p(p) {};
  ResultType parse(const std::vector<std::string>& s) const override {
    std::vector<std::string> remaining = s;
    std::vector<ElementType> values;
    auto result = p.parse(remaining);
    while (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      values.push_back(value);
      remaining = tokens;
      result = p.parse(remaining);
    }
    return ParseSuccess(values, remaining);
  }
};

// AtLeastOne(p): Like Many but requires at least one match

template<typename ParserType,
         typename ElementType = typename ParserType::ValueType,
         typename ValueType = typename std::vector<ElementType>>
struct AtLeastOne : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType p;
  explicit AtLeastOne(ParserType p) : p(p) {};
  ResultType parse(const std::vector<std::string>& s) const override {
    std::vector<std::string> remaining = s;
    std::vector<ElementType> values;
    auto result = p.parse(remaining);
    while (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      values.push_back(value);
      remaining = tokens;
      result = p.parse(remaining);
    }
    if (values.empty()) {
      return ParseFailure(result);
    } else {
      return ParseSuccess(values, remaining);
    }
  }
};

// Monadic bind. Not used in parser.cpp since we do not need the full power of monadic parsing.

template<typename ParserType,
         typename Fn,
         typename ValueType = typename std::result_of<Fn(typename ParserType::ValueType)>::type::ValueType>
struct Bind : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType m;
  Fn f;
  Bind(ParserType m, Fn f) : m(m), f(f) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    auto result = m.parse(s);
    if (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      return f(value).parse(tokens);
    } else {
      return ParseFailure(result);
    }
  }
};

template<typename ValueType>
struct Unit : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ResultType result;
  explicit Unit(ParseFailure fail) : result(fail) {}
  explicit Unit(ValueType value, std::vector<std::string> unused={}) : result(ParseSuccess(value, unused)) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    if (success(result)) {
      auto [value, unused] = ParseSuccess(result);
      unused.insert(unused.end(), s.begin(), s.end());
      return ParseSuccess(value, unused);
    } else {
      return ParseFailure(result);
    }
  }
};

// Fmap(p, f): Maps f over p, returning a new parser whose value type = return type of f

template<typename ParserType,
         typename Fn,
         typename ValueType = typename std::result_of<Fn(typename ParserType::ValueType)>::type>
struct Fmap : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType m;
  Fn f;
  explicit Fmap(ParserType m, Fn f) : m(m), f(f) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    auto result = m.parse(s);
    if (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      return ParseSuccess(f(value), tokens);
    } else {
      return ParseFailure(result);
    }
  }
};

// KeepLeft(p, q): Runs p then q and discards the result of q

template<typename Parser1Type, typename Parser2Type>
struct KeepLeft : Parser<typename Parser1Type::ValueType> {
  using ValueType = typename Parser1Type::ValueType;
  using ResultType = typename Parser<ValueType>::ResultType;
  Parser1Type p;
  Parser2Type q;
  KeepLeft(Parser1Type p, Parser2Type q) : p(p), q(q) {};
  ResultType parse(const std::vector<std::string>& s) const override {
    auto result = p.parse(s);
    if (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      auto result2 = q.parse(tokens);
      if (success(result2)) {
        auto [_, tokens2] = ParseSuccess(result2);
        return ParseSuccess(value, tokens2);
      }
      return ParseFailure(result2);
    }
    return ParseFailure(result);
  }
};

// KeepRight(p, q): Runs p then q and discards the result of p

template<typename Parser1Type, typename Parser2Type>
struct KeepRight : Parser<typename Parser2Type::ValueType> {
  using ValueType = typename Parser2Type::ValueType;
  using ResultType = typename Parser<ValueType>::ResultType;
  Parser1Type p;
  Parser2Type q;
  KeepRight(Parser1Type p, Parser2Type q) : p(p), q(q) {};
  ResultType parse(const std::vector<std::string>& s) const override {
    auto result = p.parse(s);
    if (success(result)) {
      auto [_, tokens] = ParseSuccess(result);
      return q.parse(tokens);
    }
    return ParseFailure(result);
  }
};

// AtLeastOneSepBy(p, sep): Runs p then sep alternatingly, ending with p. At least one p is
// required. Discards the results of all seps. Useful for parsing things delimited by something
// e.g. comma separated terms

template<typename ParserType, typename ParserSepType,
         typename ElementType = typename ParserType::ValueType,
         typename ValueType = typename std::vector<ElementType>>
struct AtLeastOneSepBy : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType p;
  KeepRight<ParserSepType,ParserType> q;
  explicit AtLeastOneSepBy(ParserType p, ParserSepType sep) : p(p), q(KeepRight(sep, p)) {}
  ResultType parse(const std::vector<std::string>& s) const override {
    std::vector<std::string> remaining = s;
    std::vector<ElementType> values;
    auto result = p.parse(remaining);
    while (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      values.push_back(value);
      remaining = tokens;
      result = q.parse(remaining);
    }
    if (values.empty()) {
      return ParseFailure(result);
    } else {
      return ParseSuccess(values, remaining);
    }
  }
};

// Maybe(p): Runs p at most once, wraps result in an optional

template<typename ParserType,
         typename ElementType = typename ParserType::ValueType,
         typename ValueType = typename std::optional<ElementType>>
struct Maybe : Parser<ValueType> {
  using ResultType = typename Parser<ValueType>::ResultType;
  ParserType p;
  explicit Maybe(ParserType p) : p(p) {};
  ResultType parse(const std::vector<std::string>& s) const override {
    auto result = p.parse(s);
    if (success(result)) {
      auto [value, tokens] = ParseSuccess(result);
      return ParseSuccess(std::optional<ElementType>{value}, tokens);
    }
    return ParseSuccess(std::optional<ElementType>{}, s);
  }
};

}
