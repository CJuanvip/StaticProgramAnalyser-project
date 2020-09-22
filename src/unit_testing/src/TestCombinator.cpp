#include <catch.hpp>
#include <string>
#include <vector>
#include "Combinator.h"
using namespace parser;

#define PARSE(p, s) \
    p.parse(split(s))
#define ASSERT_FAIL(p, s) \
    REQUIRE(!success(PARSE((p), (s))))
#define ASSERT_VALUE(p, s, expected) \
    REQUIRE(ParseSuccess(PARSE((p), (s))).value == expected)
#define ASSERT_REMAINING(p, s, expected) \
    REQUIRE(ParseSuccess(PARSE((p), (s))).remaining == split(expected))

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> result;
    std::string word;
    for (const auto &c : s) {
        if (c == ' ') {
            result.push_back(word);
            word.clear();
        } else {
            word.push_back(c);
        }
    }
    if (word.size() > 0) result.push_back(word);
    return result;
}

TEST_CASE("StringParser") {
    auto p = StringParser("hello");
    ASSERT_REMAINING(p, "hello world", "world");
    ASSERT_FAIL(p, "world");
}

TEST_CASE("RegexParser") {
    auto p = RegexParser("\\d+");
    ASSERT_REMAINING(p, "123 asd", "asd");
    ASSERT_FAIL(p, "123asd");
}

TEST_CASE("LazyParser") {
    auto p = LazyParser<std::string>();
    p.define(StringParser("hello"));
    ASSERT_REMAINING(p, "hello world", "world");
}

TEST_CASE("Alt") {
    auto p = Alt(StringParser("foo"), StringParser("bar"));
    ASSERT_REMAINING(p, "bar foo", "foo");
    ASSERT_FAIL(p, "baz");
}

TEST_CASE("Then") {
    auto p = Then(StringParser("foo"), StringParser("bar"));
    ASSERT_REMAINING(p, "foo bar baz", "baz");
    ASSERT_FAIL(p, "foo baz");
}

TEST_CASE("Many") {
    auto p = Many(StringParser("hello"));
    ASSERT_REMAINING(p, "hello hello hello world", "world");
    ASSERT_REMAINING(p, "world", "world");
}

TEST_CASE("AtLeastOne") {
    auto p = AtLeastOne(StringParser("hello"));
    ASSERT_REMAINING(p, "hello hello hello world", "world");
    ASSERT_FAIL(p, "world");
}

TEST_CASE("Fmap") {
    auto p = Fmap(RegexParser("\\d+"), [](auto a){ return std::stoi(a); });
    ASSERT_VALUE(p, "123", 123);
}

TEST_CASE("KeepLeft") {
    auto p = KeepLeft(StringParser("hello"), StringParser("world"));
    ASSERT_VALUE(p, "hello world", "hello");
    ASSERT_REMAINING(p, "hello world", "");
    ASSERT_FAIL(p, "hello");
}

TEST_CASE("KeepRight") {
    auto p = KeepRight(StringParser("hello"), StringParser("world"));
    ASSERT_VALUE(p, "hello world", "world");
    ASSERT_REMAINING(p, "hello world", "");
    ASSERT_FAIL(p, "hello");
}

TEST_CASE("AtLeastOneSepBy") {
    auto p = AtLeastOneSepBy(StringParser("hello"), StringParser(","));
    ASSERT_VALUE(p, "hello , hello , hello", (std::vector<std::string>{"hello", "hello", "hello"}));
    ASSERT_VALUE(p, "hello", std::vector<std::string>{"hello"});
    ASSERT_REMAINING(p, "hello world", "world");
    ASSERT_FAIL(p, "world");
}

TEST_CASE("Maybe") {
    auto p = Maybe(StringParser("hello"));
    ASSERT_REMAINING(p, "hello world", "world");
    ASSERT_REMAINING(p, "world", "world");
}