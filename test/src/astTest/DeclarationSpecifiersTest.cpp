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

TEST(DeclarationSpecifiers, constIntIsConstInteger) {
    using namespace ast;
    DeclarationSpecifiers d {
            type::Qualifier::CONST,
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } };
    auto t = d.getResolvedType();
    EXPECT_TRUE(t.isConst());
    EXPECT_FALSE(t.isVolatile());
    EXPECT_EQ(t.getSize(), 4);
    EXPECT_EQ(t.to_string(), "const int");
}

TEST(DeclarationSpecifiers, constAppliesToNonPrimitiveType) {
    using namespace ast;
    auto rec = type::structure({ { "x", type::signedInteger() } });
    DeclarationSpecifiers d {
            type::Qualifier::CONST,
            DeclarationSpecifiers { TypeSpecifier { rec, "S" } } };
    auto t = d.getResolvedType();
    EXPECT_TRUE(t.isConst());
    EXPECT_TRUE(t.isCompleteStructure());
    EXPECT_EQ(t.structureBodyIdentity(), rec.structureBodyIdentity());
}

TEST(DeclarationSpecifiers, toTypeSpecifierIdentityWhenSingleUnqualified) {
    using namespace ast;
    TypeSpecifier original { type::signedInteger(), "int" };
    DeclarationSpecifiers d { original };
    auto ts = d.toTypeSpecifier();
    EXPECT_EQ(ts.getName(), "int");
    EXPECT_TRUE(ts.getType().equivalentTo(type::signedInteger()));
    EXPECT_FALSE(ts.getType().isConst());
}

TEST(DeclarationSpecifiers, toTypeSpecifierAppliesConstKeepsStoredName) {
    using namespace ast;
    auto rec = type::structure({ { "x", type::signedInteger() } });
    DeclarationSpecifiers tagged {
            type::Qualifier::CONST,
            DeclarationSpecifiers { TypeSpecifier { rec, "S" } } };
    auto taggedTs = tagged.toTypeSpecifier();
    EXPECT_EQ(taggedTs.getName(), "S");
    EXPECT_TRUE(taggedTs.getType().isConst());
    EXPECT_TRUE(taggedTs.getType().isCompleteStructure());

    DeclarationSpecifiers untagged {
            type::Qualifier::CONST,
            DeclarationSpecifiers { TypeSpecifier { rec, "" } } };
    EXPECT_TRUE(untagged.isUntaggedCompleteRecord());
    auto untaggedTs = untagged.toTypeSpecifier();
    EXPECT_TRUE(untaggedTs.getName().empty());
    EXPECT_TRUE(untaggedTs.getType().isConst());
}

TEST(DeclarationSpecifiers, toTypeSpecifierCombinedKeywordsHaveEmptyName) {
    using namespace ast;
    DeclarationSpecifiers d {
            TypeSpecifier { type::unsignedInteger(), "unsigned" },
            DeclarationSpecifiers { TypeSpecifier { type::signedInteger(), "int" } } };
    auto ts = d.toTypeSpecifier();
    EXPECT_TRUE(ts.getName().empty());
    EXPECT_EQ(ts.getType().getSize(), 4);
    EXPECT_FALSE(ts.getType().getPrimitive().isSigned());
}

TEST(DeclarationSpecifiers, untaggedCompleteRecordIgnoresKeywordSpecs) {
    using namespace ast;
    auto rec = type::structure({ { "x", type::signedInteger() } });
    DeclarationSpecifiers untagged { TypeSpecifier { rec, "" } };
    DeclarationSpecifiers tagged { TypeSpecifier { rec, "Inner" } };
    DeclarationSpecifiers integer { TypeSpecifier { type::signedInteger(), "int" } };
    DeclarationSpecifiers unnamedInt { TypeSpecifier { type::signedInteger(), "" } };
    EXPECT_TRUE(untagged.isUntaggedCompleteRecord());
    EXPECT_FALSE(tagged.isUntaggedCompleteRecord());
    EXPECT_FALSE(integer.isUntaggedCompleteRecord());
    EXPECT_FALSE(unnamedInt.isUntaggedCompleteRecord());
}

} // namespace
