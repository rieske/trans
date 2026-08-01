#include "gtest/gtest.h"

#include "ast/DeclarationSpecifiers.h"
#include "ast/StorageSpecifier.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace {

TEST(DeclarationSpecifiers, resolveBareAndCombinedIntegers) {
    using namespace ast;
    {
        DeclarationSpecifiers d { TypeSpecifier { type::signedShort(), "short" } };
        EXPECT_TRUE(d.getResolvedType().isPrimitive());
        EXPECT_EQ(d.getResolvedType().getSize(), 2);
        EXPECT_TRUE(d.getResolvedType().getPrimitive().isSigned());
    }
    {
        DeclarationSpecifiers d { TypeSpecifier { type::unsignedInteger(), "unsigned" } };
        EXPECT_EQ(d.getResolvedType().getSize(), 4);
        EXPECT_FALSE(d.getResolvedType().getPrimitive().isSigned());
    }
    {
        // unsigned + int
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } };
        auto t = d.getResolvedType();
        EXPECT_EQ(t.getSize(), 4);
        EXPECT_FALSE(t.getPrimitive().isSigned());
    }
    {
        // unsigned + short
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers { TypeSpecifier { type::signedShort(), "short" } } };
        auto t = d.getResolvedType();
        EXPECT_EQ(t.getSize(), 2);
        EXPECT_FALSE(t.getPrimitive().isSigned());
    }
    {
        DeclarationSpecifiers d { TypeSpecifier { type::signedLong(), "long" } };
        EXPECT_EQ(d.getResolvedType().getSize(), 8);
    }
    {
        DeclarationSpecifiers d { TypeSpecifier { type::signedInteger(), "signed" } };
        EXPECT_EQ(d.getResolvedType().getSize(), 4);
        EXPECT_TRUE(d.getResolvedType().getPrimitive().isSigned());
    }
    {
        // Multi-word package name as type_name combine produces (order-independent).
        DeclarationSpecifiers d {
                TypeSpecifier { type::signedLong(), "long" },
                DeclarationSpecifiers {
                        TypeSpecifier { type::unsignedInteger(), "unsigned int" } } };
        auto t = d.getResolvedType();
        EXPECT_EQ(t.getSize(), 8);
        EXPECT_FALSE(t.getPrimitive().isSigned());
    }
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers {
                        TypeSpecifier { type::signedLong(), "long" },
                        DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } } };
        EXPECT_EQ(d.getResolvedType().getSize(), 8);
        EXPECT_FALSE(d.getResolvedType().getPrimitive().isSigned());
    }
}

TEST(DeclarationSpecifiers, isTypedefDetectsStorageClass) {
    using namespace ast;
    translation_unit::Context ctx { "t", 1 };
    DeclarationSpecifiers plain { TypeSpecifier { type::signedInteger(), "int" } };
    EXPECT_FALSE(plain.isTypedef());
    EXPECT_FALSE(plain.hasStorage(Storage::TYPEDEF));

    DeclarationSpecifiers withTypedef {
            StorageSpecifier::TYPEDEF(ctx),
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } };
    EXPECT_TRUE(withTypedef.isTypedef());
    EXPECT_TRUE(withTypedef.hasStorage(Storage::TYPEDEF));
}

} // namespace
