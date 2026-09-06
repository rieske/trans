#include "ast/TerminalSymbol.h"

#include "gtest/gtest.h"

#include <string>
#include <utility>

using ast::TerminalSymbol;

TEST(TerminalSymbol, ctorStoresLexeme) {
    TerminalSymbol t { "break", { "t.c", 1 } };
    EXPECT_EQ(t.value, "break");
}

TEST(TerminalSymbol, moveTransfersLexeme) {
    TerminalSymbol src { std::string(64, 'y'), { "t.c", 3 } };
    TerminalSymbol dst { std::move(src) };
    EXPECT_EQ(dst.value.size(), 64u);
    EXPECT_TRUE(src.value.empty());
}

TEST(TerminalSymbol, copyKeepsBothLexemes) {
    TerminalSymbol src { "name", { "t.c", 4 } };
    TerminalSymbol dst { src };
    EXPECT_EQ(src.value, "name");
    EXPECT_EQ(dst.value, "name");
}
