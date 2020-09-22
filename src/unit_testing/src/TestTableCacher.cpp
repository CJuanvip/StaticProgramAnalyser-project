#include <catch.hpp>
#include <DesignExtractor.h>
#include <ConstructAST.h>
#include <pkb/PKBTableManager.h>
#include <query_evaluator/TableCacher.h>
#include <query_evaluator/Evaluator.h>
#include <parser/AST.h>

using namespace common;
// An utility class to build a dummy PKB manager
pkb::PKBTableManager setupPKBManager_TableCacher(Program prog = buildTestCase2()) {
    using namespace ast;
    pkb::PKBTableManager mgr;

    DesignExtractor designExtractor(mgr);
    designExtractor.initialize(prog);
    designExtractor.extract();
    return mgr;
}

TEST_CASE("TableCacher getNameId, getName") {
    using namespace query_eval;
    pkb::PKBTableManager manager = setupPKBManager_TableCacher();
    TableCacher cache(manager);

    SECTION("getNameId positive") {
        auto validName = GENERATE(as<std::string>{}, "a", "b", "c", "1", "2", "3", "20", "100", "procA", "procB");
        REQUIRE(cache.getNameId(validName).has_value());
    }

    SECTION("getNameId negative") {
        auto invalidName = GENERATE(as<std::string>{}, "notInside", "-3", "z");
        REQUIRE(!cache.getNameId(invalidName).has_value());
    }

    SECTION("getName") {
        auto validName = GENERATE(as<std::string>{}, "a", "b", "c", "20", "100", "2", "procB");
        REQUIRE(cache.getName(cache.getNameId(validName).value()) == validName);
    }

}
