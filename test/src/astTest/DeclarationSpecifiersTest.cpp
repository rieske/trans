#include "gtest/gtest.h"

#include "ast/DeclarationSpecifiers.h"
#include "ast/StorageSpecifier.h"
#include "ast/TypeSpecifier.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
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
    {
        DeclarationSpecifiers d { TypeSpecifier { type::signedInt128(), "__int128" } };
        EXPECT_EQ(d.getResolvedType().getSize(), 16);
        EXPECT_TRUE(d.getResolvedType().getPrimitive().isSigned());
    }
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers { TypeSpecifier { type::signedInt128(), "__int128" } } };
        auto t = d.getResolvedType();
        EXPECT_EQ(t.getSize(), 16);
        EXPECT_FALSE(t.getPrimitive().isSigned());
    }
    {
        // unsigned + empty-name __int128 (typeof/packaged) must stay 16 bytes.
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers { TypeSpecifier { type::signedInt128(), "" } } };
        auto t = d.getResolvedType();
        EXPECT_EQ(t.getSize(), 16);
        EXPECT_FALSE(t.getPrimitive().isSigned());
    }
    {
        // unsigned + empty-name bool must stay bool, not become unsigned char.
        DeclarationSpecifiers d {
                TypeSpecifier { type::unsignedInteger(), "unsigned" },
                DeclarationSpecifiers { TypeSpecifier { type::boolean(), "" } } };
        auto t = d.getResolvedType();
        EXPECT_TRUE(type::isBoolean(t));
        EXPECT_EQ(t.getSize(), 1);
    }
}

TEST(DeclarationSpecifiers, resolveComplexSpecifiers) {
    using namespace ast;
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::complexDouble(), "_Complex" },
                DeclarationSpecifiers { TypeSpecifier { type::floating(), "float" } } };
        EXPECT_TRUE(type::isComplexFloat(d.getResolvedType()));
        EXPECT_EQ(d.getResolvedType().getSize(), 8);
        EXPECT_EQ(d.getResolvedType().getAlignment(), 4);
    }
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::floating(), "float" },
                DeclarationSpecifiers { TypeSpecifier { type::complexDouble(), "_Complex" } } };
        EXPECT_TRUE(type::isComplexFloat(d.getResolvedType()));
    }
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::complexDouble(), "_Complex" },
                DeclarationSpecifiers { TypeSpecifier { type::doubleFloating(), "double" } } };
        EXPECT_TRUE(type::isComplexDouble(d.getResolvedType()));
        EXPECT_EQ(d.getResolvedType().getSize(), 16);
    }
    {
        DeclarationSpecifiers d {
                TypeSpecifier { type::complexDouble(), "_Complex" },
                DeclarationSpecifiers {
                        TypeSpecifier { type::signedLong(), "long" },
                        DeclarationSpecifiers { TypeSpecifier { type::doubleFloating(), "double" } } } };
        EXPECT_TRUE(type::isComplexLongDouble(d.getResolvedType()));
        EXPECT_EQ(d.getResolvedType().getSize(), 32);
        EXPECT_EQ(d.getResolvedType().getAlignment(), 16);
    }
    {
        DeclarationSpecifiers d { TypeSpecifier { type::complexDouble(), "_Complex" } };
        EXPECT_TRUE(type::isComplexDouble(d.getResolvedType()));
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
    EXPECT_TRUE(untagged.isUntaggedRecordBody());
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

TEST(DeclarationSpecifiers, untaggedRecordBodyIgnoresKeywordSpecs) {
    using namespace ast;
    auto rec = type::structure({ { "x", type::signedInteger() } });
    DeclarationSpecifiers untagged { TypeSpecifier { rec, "" } };
    DeclarationSpecifiers tagged { TypeSpecifier { rec, "Inner" } };
    DeclarationSpecifiers integer { TypeSpecifier { type::signedInteger(), "int" } };
    DeclarationSpecifiers unnamedInt { TypeSpecifier { type::signedInteger(), "" } };
    EXPECT_TRUE(untagged.isUntaggedRecordBody());
    EXPECT_FALSE(tagged.isUntaggedRecordBody());
    EXPECT_FALSE(integer.isUntaggedRecordBody());
    EXPECT_FALSE(unnamedInt.isUntaggedRecordBody());
}

TEST(DeclarationSpecifiers, untaggedTentativeRecordIsAnonymousMember) {
    using namespace ast;
    type::Type rec = type::incompleteRecord();
    type::completeStructure(rec, { type::MemberSpec { "a", type::variableArray(type::signedInteger()) } });
    ASSERT_TRUE(type::isTentativeRecord(rec));
    DeclarationSpecifiers untagged { TypeSpecifier { rec, "" } };
    DeclarationSpecifiers tagged { TypeSpecifier { rec, "Inner" } };
    EXPECT_TRUE(untagged.isUntaggedRecordBody());
    EXPECT_FALSE(tagged.isUntaggedRecordBody());
}

} // namespace
