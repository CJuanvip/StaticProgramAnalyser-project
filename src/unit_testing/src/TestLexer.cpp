//
// Created by darren on 9/2/19.
//
#include "catch.hpp"
#include "Lexer.h"

std::string join(std::vector<std::string> a) {
    std::string ret;
    for (auto& s : a) ret += s + " ";
    ret.pop_back();
    return ret;
}

TEST_CASE("Tokenize with whitespaces") {
    std::string code = "procedure hello {\n a  \t\t= 3;\n\nif (a==2) then \t{\nprint b ;\n} else { read a }}";
    std::string answer = "procedure hello { a = 3 ; if ( a == 2 ) then { print b ; } else { read a } }";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("SIMPLE Greedy matching") {
    std::string s = "procedurefoo 0bar a=1 a==b a<=b a=<b";
    std::string answer = "procedurefoo 0 bar a = 1 a == b a <= b a = < b";
    auto tokens = parser::tokenizeSimple(s);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("PQL Greedy matching") {
    std::string s = R"(patternfoo Follows* fooFollows* 0bar a=1 a==b a<=b a=<b _""_ "")";
    std::string answer = R"(patternfoo Follows* fooFollows * 0 bar a = 1 a == b a <= b a = < b _" "_ " ")";
    auto tokens = parser::tokenizeQuery(s);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("PQL no relation matching in quotes") {
    std::string s = R"("Follows*a" _"Follows*a"_)";
    std::string answer = R"(" Follows * a " _" Follows * a "_)";
    auto tokens = parser::tokenizeQuery(s);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize procedure") {
    std::string code = "procedure { }";
    std::string answer = "procedure { }";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize print") {
    std::string code = "print x;";
    std::string answer = "print x ;";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize read") {
    std::string code = "read x;";
    std::string answer = "read x ;";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize call") {
    std::string code = "call anotherFunc;";
    std::string answer = "call anotherFunc ;";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize while") {
    std::string code = "while(z == 2) { c = 3; }";
    std::string answer = "while ( z == 2 ) { c = 3 ; }";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize if") {
    std::string code = "if (z == 2) then{ c = 3; } else{ c = 4; }";
    std::string answer = "if ( z == 2 ) then { c = 3 ; } else { c = 4 ; }";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize assign") {
    std::string code = "x = 2 + 3*5 /6 %7;";
    std::string answer = "x = 2 + 3 * 5 / 6 % 7 ;";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize Comparators") {
    std::string code = "while(x >= 2&& z <= 3 || (y < 4 && w > 5) && a != 6&&b== 7) { y = 3; } }";
    std::string answer = "while ( x >= 2 && z <= 3 || ( y < 4 && w > 5 ) && a != 6 && b == 7 ) { y = 3 ; } }";
    auto tokens = parser::tokenizeSimple(code);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL synonyms") {
    std::string pql = "stmt s; assign a; variable v; constant c;";
    std::string answer = "stmt s ; assign a ; variable v ; constant c ;";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Follows") {
    std::string pql = "stmt s; Select s such that Follows(3, s)";
    std::string answer = "stmt s ; Select s such that Follows ( 3 , s )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Follows*") {
    std::string pql = "stmt s; Select s such that Follows*(3, s)";
    std::string answer = "stmt s ; Select s such that Follows* ( 3 , s )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Parent") {
    std::string pql = "stmt s; Select s such that Parent(s, 6)";
    std::string answer = "stmt s ; Select s such that Parent ( s , 6 )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Parent*") {
    std::string pql = "stmt s; Select s such that Parent*(s, 6)";
    std::string answer = "stmt s ; Select s such that Parent* ( s , 6 )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Modifies") {
    std::string pql = "stmt s; variable v; Select s such that Modifies(s, v)";
    std::string answer = "stmt s ; variable v ; Select s such that Modifies ( s , v )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Uses") {
    std::string pql = "stmt s; variable v; Select s such that Uses(s, v)";
    std::string answer = "stmt s ; variable v ; Select s such that Uses ( s , v )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}

TEST_CASE("Tokenize PQL Pattern") {
    std::string pql = "assign a; while w; Select w such that Parent*(w, a) pattern a(\"count\", _)";
    std::string answer = "assign a ; while w ; Select w such that Parent* ( w , a ) pattern a ( \" count \" , _ )";
    auto tokens = parser::tokenizeQuery(pql);
    REQUIRE(answer == join(tokens));
}
