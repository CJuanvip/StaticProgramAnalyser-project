#include <catch.hpp>
#include "Parser.h"

TEST_CASE("Strip comments") {
    using namespace parser;
    auto s = "line1//\nline2//comment\nline3//\r\nline4\\\\comment\rcomment\r\nno newline//";
    auto expected = "line1\nline2\nline3\r\nline4\r\nno newline";
    REQUIRE(stripComments(s) == expected);
}
