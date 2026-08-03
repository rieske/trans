#include "gtest/gtest.h"

#include "ast/FunctionCall.h"
#include "ast/IdentifierExpression.h"
#include "ast/OffsetofExpression.h"
#include "ast/TypeName.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

using namespace ast;

TEST(FunctionCall, builtinTypeNameIsVaArgOnly) {
    translation_unit::Context ctx { "t", 1 };
    auto call = std::make_unique<FunctionCall>(
            std::make_unique<IdentifierExpression>("__builtin_va_arg", ctx));
    EXPECT_EQ(call->builtinTypeName(), nullptr);
    call->setBuiltinTypeName(TypeName { TypeSpecifier { type::signedInteger(), "int" }, nullptr });
    ASSERT_NE(call->builtinTypeName(), nullptr);
    EXPECT_TRUE(call->builtinTypeName()->spec.getType().equivalentTo(type::signedInteger()));
    long folded = 0;
    EXPECT_FALSE(call->evaluateConstant(folded));
}

TEST(OffsetofExpression, typeNameMemberAndFoldedInteger) {
    translation_unit::Context ctx { "t", 1 };
    OffsetofExpression expr {
            TypeName { TypeSpecifier { type::signedInteger(), "int" }, nullptr },
            "hash",
            ctx };
    EXPECT_TRUE(expr.getTypeName().spec.getType().equivalentTo(type::signedInteger()));
    EXPECT_EQ(expr.getMemberName(), "hash");
    long value = 0;
    EXPECT_FALSE(expr.evaluateConstant(value));
    expr.setFoldedInteger(32);
    ASSERT_TRUE(expr.evaluateConstant(value));
    EXPECT_EQ(value, 32);
}
