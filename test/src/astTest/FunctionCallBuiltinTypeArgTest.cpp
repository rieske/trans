#include "gtest/gtest.h"

#include "ast/FunctionCall.h"
#include "ast/IdentifierExpression.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

using namespace ast;

TEST(FunctionCall, builtinTypeArgumentLivesOnCallNode) {
    translation_unit::Context ctx { "t", 1 };
    auto call = std::make_unique<FunctionCall>(
            std::make_unique<IdentifierExpression>("__builtin_va_arg", ctx));
    EXPECT_EQ(call->builtinTypeArgument(), nullptr);
    call->setBuiltinTypeArgument(type::signedInteger());
    ASSERT_NE(call->builtinTypeArgument(), nullptr);
    EXPECT_TRUE(call->builtinTypeArgument()->equivalentTo(type::signedInteger()));
}
