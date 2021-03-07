#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/DeclarationSpecifiers.h"
#include "semantic_analyzer/SemanticXmlOutputVisitor.h"

namespace {

using namespace testing;
using namespace ast;
using namespace semantic_analyzer;

translation_unit::Context context { "test.c", 42 };

TEST(DeclarationSpecifiers, isConstructedUsingTypeSpecifier) {
    DeclarationSpecifiers declSpecs { TypeSpecifier { type::signedInteger(), "int" } };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), SizeIs(1));
    EXPECT_THAT(declSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), IsEmpty());
}

TEST(DeclarationSpecifiers, isConstructedUsingTypeQualifier) {
    DeclarationSpecifiers declSpecs { type::Qualifier::CONST };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getTypeQualifiers(), ElementsAre(type::Qualifier::CONST));
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), IsEmpty());
}

TEST(DeclarationSpecifiers, isConstructedUsingStorageSpecifier) {
    DeclarationSpecifiers declSpecs { StorageSpecifier::STATIC(context) };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC(context)));
}

TEST(DeclarationSpecifiers, canBeChainConstructed) {
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { type::signedInteger(), "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { type::signedInteger(), "int" }, intDeclSpecs };
    EXPECT_THAT(intIntDeclSpecs.getTypeSpecifiers(), SizeIs(2));
    EXPECT_THAT(intIntDeclSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(intIntDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { type::voidType(), "void" }, intIntDeclSpecs };
    EXPECT_THAT(intIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(intIntVoidDeclSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(intIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers constIntIntVoidDeclSpecs { type::Qualifier::CONST, intIntVoidDeclSpecs };
    EXPECT_THAT(constIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(constIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(type::Qualifier::CONST));
    EXPECT_THAT(constIntIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers constConstIntIntVoidDeclSpecs { type::Qualifier::CONST, constIntIntVoidDeclSpecs };
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(type::Qualifier::CONST, type::Qualifier::CONST));
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers staticConstConstIntIntVoidDeclSpecs { StorageSpecifier::STATIC(context), constConstIntIntVoidDeclSpecs };
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(type::Qualifier::CONST, type::Qualifier::CONST));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC(context)));

    DeclarationSpecifiers autoStaticConstConstIntIntVoidDeclSpecs { StorageSpecifier::AUTO(context), staticConstConstIntIntVoidDeclSpecs };
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(type::Qualifier::CONST, type::Qualifier::CONST));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(),
            ElementsAre(StorageSpecifier::STATIC(context), StorageSpecifier::AUTO(context)));
}

TEST(SemanticXmlOutputVisitor, outputsDeclarationSpecifiersAsXml) {
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { type::signedInteger(), "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { type::signedInteger(), "int" }, intDeclSpecs };

    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { type::voidType(), "void" }, intIntDeclSpecs };
    DeclarationSpecifiers constIntIntVoidDeclSpecs { type::Qualifier::CONST, intIntVoidDeclSpecs };
    DeclarationSpecifiers constConstIntIntVoidDeclSpecs { type::Qualifier::CONST, constIntIntVoidDeclSpecs };
    DeclarationSpecifiers staticConstConstIntIntVoidDeclSpecs { StorageSpecifier::STATIC(context), constConstIntIntVoidDeclSpecs };
    DeclarationSpecifiers autoStaticConstConstIntIntVoidDeclSpecs { StorageSpecifier::AUTO(context), staticConstConstIntIntVoidDeclSpecs };

    std::ostringstream outputStream;
    SemanticXmlOutputVisitor outputVisitor { &outputStream };
    autoStaticConstConstIntIntVoidDeclSpecs.accept(outputVisitor);

    EXPECT_THAT(outputStream.str(), StrEq("<declarationSpecifiers>\n"
            "  <typeSpecifier>int</typeSpecifier>\n"
            "  <typeSpecifier>int</typeSpecifier>\n"
            "  <typeSpecifier>void</typeSpecifier>\n"
            "  <typeQualifier>const</typeQualifier>\n"
            "  <typeQualifier>const</typeQualifier>\n"
            "  <storageSpecifier>static</storageSpecifier>\n"
            "  <storageSpecifier>auto</storageSpecifier>\n"
            "</declarationSpecifiers>\n"));
}

}
