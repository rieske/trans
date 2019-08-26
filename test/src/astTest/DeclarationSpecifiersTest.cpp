#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/types/IntegralType.h"
#include "src/ast/DeclarationSpecifiers.h"
#include "src/semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "src/types/VoidType.h"

namespace {

using namespace testing;
using namespace ast;
using namespace semantic_analyzer;

translation_unit::Context context { "test.c", 42 };

TEST(DeclarationSpecifiers, isConstructedUsingTypeSpecifier) {
    auto integer = IntegralType::newSignedInteger();
    DeclarationSpecifiers declSpecs { TypeSpecifier { *integer, "int" } };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), SizeIs(1));
    EXPECT_THAT(declSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), IsEmpty());
}

TEST(DeclarationSpecifiers, isConstructedUsingTypeQualifier) {
    DeclarationSpecifiers declSpecs { TypeQualifier::CONST };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST));
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), IsEmpty());
}

TEST(DeclarationSpecifiers, isConstructedUsingStorageSpecifier) {
    DeclarationSpecifiers declSpecs { StorageSpecifier::STATIC(context) };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC(context)));
}

TEST(DeclarationSpecifiers, canBeChainConstructed) {
    auto integer = IntegralType::newSignedInteger();
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { *integer, "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { *integer, "int" }, intDeclSpecs };
    EXPECT_THAT(intIntDeclSpecs.getTypeSpecifiers(), SizeIs(2));
    EXPECT_THAT(intIntDeclSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(intIntDeclSpecs.getStorageSpecifiers(), IsEmpty());

    VoidType voidType;
    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { voidType, "void" }, intIntDeclSpecs };
    EXPECT_THAT(intIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(intIntVoidDeclSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(intIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers constIntIntVoidDeclSpecs { TypeQualifier::CONST, intIntVoidDeclSpecs };
    EXPECT_THAT(constIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(constIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST));
    EXPECT_THAT(constIntIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers constConstIntIntVoidDeclSpecs { TypeQualifier::CONST, constIntIntVoidDeclSpecs };
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST, TypeQualifier::CONST));
    EXPECT_THAT(constConstIntIntVoidDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers staticConstConstIntIntVoidDeclSpecs { StorageSpecifier::STATIC(context), constConstIntIntVoidDeclSpecs };
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST, TypeQualifier::CONST));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC(context)));

    DeclarationSpecifiers autoStaticConstConstIntIntVoidDeclSpecs { StorageSpecifier::AUTO(context), staticConstConstIntIntVoidDeclSpecs };
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST, TypeQualifier::CONST));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(),
            ElementsAre(StorageSpecifier::STATIC(context), StorageSpecifier::AUTO(context)));
}

TEST(SemanticXmlOutputVisitor, outputsDeclarationSpecifiersAsXml) {
    auto integer = IntegralType::newSignedInteger();
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { *integer, "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { *integer, "int" }, intDeclSpecs };
    VoidType voidType;
    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { voidType, "void" }, intIntDeclSpecs };
    DeclarationSpecifiers constIntIntVoidDeclSpecs { TypeQualifier::CONST, intIntVoidDeclSpecs };
    DeclarationSpecifiers constConstIntIntVoidDeclSpecs { TypeQualifier::CONST, constIntIntVoidDeclSpecs };
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
