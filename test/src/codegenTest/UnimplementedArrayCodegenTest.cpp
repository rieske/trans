#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/ArrayAccess.h"
#include "ast/ArrayDeclarator.h"
#include "ast/Constant.h"
#include "ast/ConstantExpression.h"
#include "ast/Identifier.h"
#include "ast/IdentifierExpression.h"
#include "ast/TerminalSymbol.h"
#include "types/Type.h"
#include "codegen/CodeGeneratingVisitor.h"
#include "symbols/AnnotationStore.h"
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

TEST(CodeGeneratingVisitor, arrayAccessWithoutSymbolsIsNoOp) {
    // Without semantic analysis, ArrayAccess has no lvalue/result temps — codegen skips IR.
    ArrayAccess access {
            std::make_unique<IdentifierExpression>("a", testContext()),
            std::make_unique<IdentifierExpression>("i", testContext())
    };
    symbols::AnnotationStore store;
    CodeGeneratingVisitor visitor(store);
    EXPECT_NO_THROW(access.accept(visitor));
    EXPECT_THAT(visitor.takeIr().procedures, IsEmpty());
}

TEST(CodeGeneratingVisitor, arrayDeclaratorIsNoOp) {
    // Sized arrays are typed in semantic analysis; declarator codegen emits no IR.
    ArrayDeclarator declarator {
            std::make_unique<Identifier>(TerminalSymbol { "id", "a", testContext() }),
            std::make_unique<ConstantExpression>(
                    Constant { "3", type::signedInteger(), testContext() })
    };
    symbols::AnnotationStore store;
    CodeGeneratingVisitor visitor(store);
    EXPECT_NO_THROW(declarator.accept(visitor));
    EXPECT_THAT(visitor.takeIr().procedures, IsEmpty());
}

} // namespace
