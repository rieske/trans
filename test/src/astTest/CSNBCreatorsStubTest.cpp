#include "gtest/gtest.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "scanner/LexicalSession.h"
#include "ast/CSNB_Internal.h"

namespace {

// Remaining type stubs still throw; integer type-specs are implemented (Phase 1).
TEST(CSNBCreators, unimplementedTypeStubsThrow) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    EXPECT_THROW(ast::typedefName(context), std::runtime_error);
    EXPECT_THROW(ast::enumType(context), std::runtime_error);
}

TEST(CSNBCreators, notImplementedYetFactoryThrows) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    auto stub = ast::notImplementedYet("feature X");
    EXPECT_THROW(stub(context), std::runtime_error);
}

TEST(CSNBCreators, doNothingIsNoOp) {
    scanner::LexicalSession session;
    ast::AbstractSyntaxTreeBuilderContext context{session};
    EXPECT_NO_THROW(ast::doNothing(context));
}

} // namespace
