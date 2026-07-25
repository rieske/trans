#include "gtest/gtest.h"

#include "ast/AbstractSyntaxTreeBuilderContext.h"
#include "ast/CSNB_Internal.h"

namespace {

// Cover not-yet-implemented type stubs (always throw regardless of later features).
TEST(CSNBCreators, unimplementedTypeStubsThrow) {
    ast::AbstractSyntaxTreeBuilderContext context;
    EXPECT_THROW(ast::shortType(context), std::runtime_error);
    EXPECT_THROW(ast::longType(context), std::runtime_error);
    EXPECT_THROW(ast::doubleType(context), std::runtime_error);
    EXPECT_THROW(ast::signedType(context), std::runtime_error);
    EXPECT_THROW(ast::unsignedType(context), std::runtime_error);
    EXPECT_THROW(ast::typedefName(context), std::runtime_error);
    EXPECT_THROW(ast::enumType(context), std::runtime_error);
}

TEST(CSNBCreators, notImplementedYetFactoryThrows) {
    ast::AbstractSyntaxTreeBuilderContext context;
    auto stub = ast::notImplementedYet("feature X");
    EXPECT_THROW(stub(context), std::runtime_error);
}

TEST(CSNBCreators, doNothingIsNoOp) {
    ast::AbstractSyntaxTreeBuilderContext context;
    EXPECT_NO_THROW(ast::doNothing(context));
}

} // namespace
