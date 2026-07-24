#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/ArrayAccess.h"
#include "ast/ArrayDeclarator.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/TerminalSymbol.h"
#include "codegen/CodeGeneratingVisitor.h"
#include "translation_unit/Context.h"

#include <memory>
#include <stdexcept>
#include <string>

namespace {

using namespace testing;
using namespace ast;
using namespace codegen;

translation_unit::Context testContext() {
    return { "test", 1 };
}

TEST(CodeGeneratingVisitor, arrayAccessIsNotImplemented) {
    ArrayAccess access {
            std::make_unique<IdentifierExpression>("a", testContext()),
            std::make_unique<IdentifierExpression>("i", testContext())
    };
    CodeGeneratingVisitor visitor;
    try {
        access.accept(visitor);
        FAIL() << "expected array access codegen to throw";
    } catch (const std::runtime_error& error) {
        EXPECT_THAT(std::string(error.what()), HasSubstr("array access is not implemented"));
    }
}

TEST(CodeGeneratingVisitor, arrayDeclaratorIsNoOp) {
    // Sized arrays are typed in semantic analysis; declarator codegen emits no IR.
    ArrayDeclarator declarator {
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", testContext() }),
            std::make_unique<IdentifierExpression>("n", testContext())
    };
    CodeGeneratingVisitor visitor;
    EXPECT_NO_THROW(declarator.accept(visitor));
    EXPECT_THAT(visitor.getQuadruples().size(), Eq(0u));
}

} // namespace
