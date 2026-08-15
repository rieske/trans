#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "codegen/IrStringTable.h"

#include <stdexcept>
#include <string>
#include <string_view>

namespace {

using namespace testing;
using namespace codegen;

TEST(IrStringTable, internIsStableAndDense) {
    IrStringTable table;
    const int a = table.intern("t0");
    const int b = table.intern("t1");
    const int a2 = table.intern("t0");

    EXPECT_THAT(a, Eq(0));
    EXPECT_THAT(b, Eq(1));
    EXPECT_THAT(a2, Eq(a));
    EXPECT_THAT(table.get(a), StrEq("t0"));
    EXPECT_THAT(table.get(b), StrEq("t1"));
    EXPECT_THAT(table.size(), Eq(2));
}

TEST(IrStringTable, internLookupIsStableAcrossStringAndStringView) {
    IrStringTable table;
    const int id = table.intern(std::string { "shared" });
    std::string other = "shared";
    EXPECT_THAT(table.intern(std::string_view { other }), Eq(id));
    EXPECT_THAT(table.find("shared"), Eq(id));
    EXPECT_THAT(table.size(), Eq(1));
}

TEST(IrStringTable, internAcceptsStringViewWithoutOwningCallersBuffer) {
    IrStringTable table;
    std::string name = "scratch";
    const int id = table.intern(name);
    name = "mutated";
    EXPECT_THAT(table.get(id), StrEq("scratch"));
}

TEST(IrStringTable, internEmptyIsNoSymbol) {
    IrStringTable table;
    EXPECT_THAT(table.intern(""), Eq(kNoSymbol));
    EXPECT_THAT(table.size(), Eq(0));
}

TEST(IrStringTable, getRejectsMissingId) {
    IrStringTable table;
    table.intern("x");
    EXPECT_THROW(table.get(-1), std::logic_error);
    EXPECT_THROW(table.get(1), std::logic_error);
}

TEST(IrStringTable, findReturnsInternedIdOrNoSymbol) {
    IrStringTable table;
    const int x = table.intern("x");
    EXPECT_THAT(table.find("x"), Eq(x));
    EXPECT_THAT(table.find("missing"), Eq(kNoSymbol));
    EXPECT_THAT(table.find(""), Eq(kNoSymbol));
}

TEST(IrStringTable, requireReturnsInternedIdAndRejectsMissing) {
    IrStringTable table;
    const int x = table.intern("x");
    EXPECT_THAT(table.require("x"), Eq(x));
    EXPECT_THROW(table.require("missing"), std::logic_error);
    EXPECT_THROW(table.require(""), std::logic_error);
}

} // namespace
