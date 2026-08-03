#include "gtest/gtest.h"

#include <memory>
#include <stdexcept>
#include <vector>

#include "ast/Block.h"
#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"
#include "ast/FormalArgument.h"
#include "ast/FunctionDeclarator.h"
#include "ast/FunctionDefinition.h"
#include "ast/Identifier.h"
#include "ast/NestedDeclarator.h"
#include "ast/Pointer.h"
#include "ast/TerminalSymbol.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"

using namespace ast;

namespace {

translation_unit::Context ctx() {
    return { "t", 1 };
}

TerminalSymbol id(const std::string& name) {
    return { "id", name, ctx() };
}

DeclarationSpecifiers intSpecs() {
    return DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } };
}

std::unique_ptr<Block> emptyBody() {
    return std::make_unique<Block>(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> {});
}

FormalArgument namedIntParam(const std::string& name) {
    return FormalArgument { intSpecs(), std::make_unique<Declarator>(std::make_unique<Identifier>(id(name))) };
}

TEST(FunctionDefinition, innermostParameterNamesForFunctionReturningFunctionPointer) {
    FormalArguments innerArgs;
    innerArgs.push_back(namedIntParam("which"));
    auto inner = std::make_unique<FunctionDeclarator>(
            std::make_unique<Identifier>(id("pick_fp")), std::move(innerArgs));

    std::vector<Pointer> stars;
    stars.emplace_back();
    auto paren = std::make_unique<NestedDeclarator>(
            std::make_unique<Declarator>(std::move(inner), std::move(stars)));

    FormalArguments outerArgs;
    outerArgs.push_back(FormalArgument { intSpecs() });
    auto outer = std::make_unique<FunctionDeclarator>(std::move(paren), std::move(outerArgs));

    FunctionDefinition def { intSpecs(), std::make_unique<Declarator>(std::move(outer)), emptyBody() };

    const auto names = def.definedFunctionParameterNames();
    ASSERT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "which");
}

TEST(FunctionDefinition, noArgFunctionHasEmptyParameterNames) {
    auto fn = std::make_unique<FunctionDeclarator>(std::make_unique<Identifier>(id("f")));
    FunctionDefinition def { intSpecs(), std::make_unique<Declarator>(std::move(fn)), emptyBody() };

    EXPECT_TRUE(def.definedFunctionParameterNames().empty());
}

TEST(FunctionDefinition, nonFunctionDeclaratorIsNotADefinedFunction) {
    FunctionDefinition def {
            intSpecs(),
            std::make_unique<Declarator>(std::make_unique<Identifier>(id("x"))),
            emptyBody() };
    EXPECT_THROW(def.definedFunctionParameterNames(), std::logic_error);
}

} // namespace
