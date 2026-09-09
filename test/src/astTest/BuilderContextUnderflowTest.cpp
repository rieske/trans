#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/Block.h"
#include "ast/IdentifierExpression.h"
#include "ast/ReturnStatement.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

#include <memory>
#include <stdexcept>
#include <string>

using testing::AllOf;
using testing::HasSubstr;
using testing::ThrowsMessage;

namespace {

translation_unit::Context ctx() {
    return { "t.c", 1 };
}

// Every pop reports the stack it underflowed rather than reading std::stack::top() empty.
#define EXPECT_UNDERFLOW(call, stackName) EXPECT_THAT([&] { call; }, \
        ThrowsMessage<std::logic_error>(AllOf(HasSubstr("internal compiler error"), HasSubstr(stackName))))

TEST(BuilderContext, popsOnEmptyStacksThrowNamingTheStack) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    EXPECT_UNDERFLOW(context.popTerminal(), "terminal");
    EXPECT_UNDERFLOW(context.popTypeSpecifier(), "type specifier");
    EXPECT_UNDERFLOW(context.popStorageSpecifier(), "storage specifier");
    EXPECT_UNDERFLOW(context.popTypeQualifier(), "type qualifier");
    EXPECT_UNDERFLOW(context.popTypeQualifierList(), "type qualifier list");
    EXPECT_UNDERFLOW(context.popConstant(), "constant");
    EXPECT_UNDERFLOW(context.popExpression(), "expression");
    EXPECT_UNDERFLOW(context.popActualArgumentsList(), "actual argument list");
    EXPECT_UNDERFLOW(context.popPointers(), "pointer");
    EXPECT_UNDERFLOW(context.popStatement(), "statement");
    EXPECT_UNDERFLOW(context.popDirectDeclarator(), "direct declarator");
    EXPECT_UNDERFLOW(context.popDeclarator(), "declarator");
    EXPECT_UNDERFLOW(context.popInitializedDeclarator(), "initialized declarator");
    EXPECT_UNDERFLOW(context.popInitializedDeclarators(), "initialized declarator list");
    EXPECT_UNDERFLOW(context.popDeclarationList(), "declaration list");
    EXPECT_UNDERFLOW(context.popFormalArgument(), "formal argument");
    EXPECT_UNDERFLOW(context.popFormalArguments(), "formal argument list");
    EXPECT_UNDERFLOW(context.popArgumentsDeclaration(), "arguments declaration");
    EXPECT_UNDERFLOW(context.popDeclarationSpecifiers(), "declaration specifiers");
    EXPECT_UNDERFLOW(context.popDeclaration(), "declaration");
    EXPECT_UNDERFLOW(context.popStatementList(), "statement list");
    EXPECT_UNDERFLOW(context.popExternalDeclaration(), "external declaration");
    EXPECT_UNDERFLOW(context.popIsUnion(), "struct or union");
    EXPECT_UNDERFLOW(context.popStructMemberList(), "struct member list");
    EXPECT_UNDERFLOW(context.takeStructDeclarators(), "struct declarator list");
    EXPECT_UNDERFLOW(context.popStructDeclaratorList(), "struct declarator list");
    EXPECT_UNDERFLOW(context.popGenericAssociation(), "generic association");
    EXPECT_UNDERFLOW(context.popGenericAssocList(), "generic association list");
}

TEST(BuilderContext, statementDerivedPopsThrowThroughPopStatement) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    EXPECT_UNDERFLOW(context.popAsStatement(), "statement");
    EXPECT_UNDERFLOW(context.popBlock(), "statement");
}

TEST(BuilderContext, addToListOnEmptyStackThrowsNamingTheStack) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    EXPECT_UNDERFLOW(context.addToActualArgumentsList(nullptr), "actual argument list");
    EXPECT_UNDERFLOW(context.pointerToPointer(ast::Pointer {}), "pointer");
    EXPECT_UNDERFLOW(context.addToTypeQualifierList(type::Qualifier::CONST), "type qualifier list");
    EXPECT_UNDERFLOW(
            context.addToStatementList(ast::BlockItem { std::make_unique<ast::ReturnStatement>() }),
            "statement list");
    EXPECT_UNDERFLOW(context.addStructMember("m", type::signedInteger()), "struct member list");
    EXPECT_UNDERFLOW(context.addStructEnumerators({}), "struct member list");
    EXPECT_UNDERFLOW(context.addStructDeclarator(nullptr), "struct declarator list");
    EXPECT_UNDERFLOW(context.addGenericAssociation(ast::GenericAssociation {}),
            "generic association list");
}

// Not underflow: these two tolerate an empty stack because the grammar can reduce an
// initializer element or a designator with no list open. Pinned so the guard does not
// tighten them by accident.
TEST(BuilderContext, tolerantAccessorsStayTolerant) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    EXPECT_TRUE(context.popInitializerList().empty());

    std::vector<ast::DesignatorStep> steps;
    steps.push_back(ast::DesignatorStep::member("unused"));
    context.takePendingDesignator(steps);
    EXPECT_TRUE(steps.empty());

    context.addInitializerElement(ast::InitializerElement { nullptr });
    EXPECT_EQ(context.popInitializerList().size(), 1u);
}

// The declarator list is a frame per struct body, drained once per member declaration.
// Nesting is what a struct defined inside a member declaration depends on.
TEST(BuilderContext, structDeclaratorFramesNest) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    context.newStructDeclaratorList();
    context.addStructDeclarator(nullptr);

    context.newStructDeclaratorList();
    context.addStructDeclarator(nullptr);
    EXPECT_EQ(context.takeStructDeclarators().size(), 1u);
    context.popStructDeclaratorList();

    // The outer declarator survived the inner body, and the frame outlives the drain.
    context.addStructDeclarator(nullptr);
    EXPECT_EQ(context.takeStructDeclarators().size(), 2u);
    EXPECT_TRUE(context.takeStructDeclarators().empty());
    context.popStructDeclaratorList();
}

// Closing a body whose declarators were never drained would silently drop struct members,
// which is the failure this frame exists to prevent. It is reported, not swallowed.
TEST(BuilderContext, closingAnUndrainedStructDeclaratorFrameThrows) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    context.newStructDeclaratorList();
    context.addStructDeclarator(nullptr);
    EXPECT_THAT([&] { context.popStructDeclaratorList(); },
            ThrowsMessage<std::logic_error>(HasSubstr("undrained declarators")));
}

// The guard must not cost the happy path: push then pop still round-trips.
TEST(BuilderContext, pushThenPopStillRoundTrips) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context { session };

    context.pushTerminal(ast::TerminalSymbol { ";", ctx() });
    EXPECT_EQ(context.popTerminal().value, ";");

    context.pushExpression(std::make_unique<ast::IdentifierExpression>("x", ctx()));
    EXPECT_NE(context.popExpression(), nullptr);

    context.newTypeQualifierList(type::Qualifier::CONST);
    context.addToTypeQualifierList(type::Qualifier::VOLATILE);
    EXPECT_EQ(context.popTypeQualifierList().size(), 2u);

    context.pushIsUnion(true);
    EXPECT_TRUE(context.popIsUnion());
}

} // namespace
