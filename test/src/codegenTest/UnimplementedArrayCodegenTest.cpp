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

TEST(CodeGeneratingVisitor, arrayDeclaratorIsNotImplemented) {
    ArrayDeclarator declarator {
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", testContext() }),
            std::make_unique<IdentifierExpression>("n", testContext())
    };
    CodeGeneratingVisitor visitor;
    try {
        declarator.accept(visitor);
        FAIL() << "expected array declarator codegen to throw";
    } catch (const std::runtime_error& error) {
        EXPECT_THAT(std::string(error.what()), HasSubstr("array declarators is not implemented"));
    }
}

} // namespace
