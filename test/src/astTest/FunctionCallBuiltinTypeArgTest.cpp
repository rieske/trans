#include "gtest/gtest.h"

#include "ast/Constant.h"
#include "ast/ConstantExpression.h"
#include "ast/FunctionCall.h"
#include "ast/IdentifierExpression.h"
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
    EXPECT_FALSE(call->foldToHostLong(folded));
    EXPECT_FALSE(call->isGnuConstantP());
}

TEST(FunctionCall, gnuConstantPFoldsOneArg) {
    translation_unit::Context ctx { "t", 1 };
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::make_unique<ConstantExpression>(
            Constant { "1", type::signedInteger(), ctx }));
    FunctionCall call {
            std::make_unique<IdentifierExpression>("__builtin_constant_p", ctx),
            std::move(args) };
    EXPECT_TRUE(call.isGnuConstantP());
    long folded = 0;
    ASSERT_TRUE(call.foldToHostLong(folded));
    EXPECT_EQ(folded, 1);
}
