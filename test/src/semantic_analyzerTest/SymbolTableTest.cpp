#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "semantic_analyzer/SymbolTable.h"
#include "types/Type.h"

namespace {

using namespace testing;
using namespace semantic_analyzer;

// Empty formal names (abstract parameters) must still produce one symbol-table
// argument slot per declared parameter. Silent drop of the second empty name
// collapses the callee ABI while the function type arity stays correct.

TEST(SymbolTable, abstractArgumentNamesPreserveArity) {
    SymbolTable table;
    translation_unit::Context ctx { "test", 1 };
    auto functionType = type::function(type::signedInteger(), { type::signedInteger(), type::signedInteger() });
    table.insertFunction("add", functionType.getFunction(), ctx);

    table.startFunction("add", { "", "" });

    EXPECT_THAT(table.getCurrentScopeArguments().size(), Eq(2u));
}

} // namespace
