#include "gtest/gtest.h"

#include <memory>

#include "ast/ArrayDeclarator.h"
#include "ast/FunctionDeclarator.h"
#include "ast/Identifier.h"

namespace {

using namespace ast;

TerminalSymbol name(const char* id) {
    return TerminalSymbol { id, { "t.c", 1 } };
}

TEST(DirectDeclarator, identifierHasNoArray) {
    Identifier id { name("a") };
    EXPECT_FALSE(id.hasArrayDeclarator());
}

TEST(DirectDeclarator, identifierNameIsStored) {
    Identifier id { name("foo") };
    EXPECT_EQ(id.getName(), "foo");
    EXPECT_EQ(&id.getName(), &id.getName());
}

TEST(DirectDeclarator, functionDeclaratorHasNoArray) {
    FunctionDeclarator fn { std::make_unique<Identifier>(name("f")) };
    EXPECT_FALSE(fn.hasArrayDeclarator());
}

TEST(DirectDeclarator, functionDeclaratorWalksNestedArray) {
    auto array = std::make_unique<ArrayDeclarator>(
            std::make_unique<Identifier>(name("a")), nullptr);
    FunctionDeclarator fn { std::move(array) };
    EXPECT_TRUE(fn.hasArrayDeclarator());
}

} // namespace
