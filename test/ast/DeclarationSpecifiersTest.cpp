#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/DeclarationSpecifiers.h"
#include "ast/types/BaseType.h"

#include "semantic_analyzer/SemanticXmlOutputVisitor.h"

#include <sstream>

namespace {

using namespace testing;
using namespace ast;
using namespace semantic_analyzer;

TEST(DeclarationSpecifiers, isConstructedUsingTypeSpecifier) {
    DeclarationSpecifiers declSpecs { TypeSpecifier { BaseType::newInteger(), "int" } };

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
    DeclarationSpecifiers declSpecs { StorageSpecifier::STATIC };

    EXPECT_THAT(declSpecs.getTypeSpecifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(declSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC));
}

TEST(DeclarationSpecifiers, canBeChainConstructed) {
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { BaseType::newInteger(), "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { BaseType::newInteger(), "int" }, intDeclSpecs };
    EXPECT_THAT(intIntDeclSpecs.getTypeSpecifiers(), SizeIs(2));
    EXPECT_THAT(intIntDeclSpecs.getTypeQualifiers(), IsEmpty());
    EXPECT_THAT(intIntDeclSpecs.getStorageSpecifiers(), IsEmpty());

    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { BaseType::newVoid(), "void" }, intIntDeclSpecs };
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

    DeclarationSpecifiers staticConstConstIntIntVoidDeclSpecs { StorageSpecifier::STATIC, constConstIntIntVoidDeclSpecs };
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST, TypeQualifier::CONST));
    EXPECT_THAT(staticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC));

    DeclarationSpecifiers autoStaticConstConstIntIntVoidDeclSpecs { StorageSpecifier::AUTO, staticConstConstIntIntVoidDeclSpecs };
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeSpecifiers(), SizeIs(3));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getTypeQualifiers(), ElementsAre(TypeQualifier::CONST, TypeQualifier::CONST));
    EXPECT_THAT(autoStaticConstConstIntIntVoidDeclSpecs.getStorageSpecifiers(), ElementsAre(StorageSpecifier::STATIC, StorageSpecifier::AUTO));
}

TEST(SemanticXmlOutputVisitor, outputsDeclarationSpecifiersAsXml) {
    DeclarationSpecifiers intDeclSpecs { TypeSpecifier { BaseType::newInteger(), "int" } };
    DeclarationSpecifiers intIntDeclSpecs { TypeSpecifier { BaseType::newInteger(), "int" }, intDeclSpecs };
    DeclarationSpecifiers intIntVoidDeclSpecs { TypeSpecifier { BaseType::newVoid(), "void" }, intIntDeclSpecs };
    DeclarationSpecifiers constIntIntVoidDeclSpecs { TypeQualifier::CONST, intIntVoidDeclSpecs };
    DeclarationSpecifiers constConstIntIntVoidDeclSpecs { TypeQualifier::CONST, constIntIntVoidDeclSpecs };
    DeclarationSpecifiers staticConstConstIntIntVoidDeclSpecs { StorageSpecifier::STATIC, constConstIntIntVoidDeclSpecs };
    DeclarationSpecifiers autoStaticConstConstIntIntVoidDeclSpecs { StorageSpecifier::AUTO, staticConstConstIntIntVoidDeclSpecs };

    std::ostringstream outputStream;
    SemanticXmlOutputVisitor outputVisitor { &outputStream };
    autoStaticConstConstIntIntVoidDeclSpecs.accept(outputVisitor);

    EXPECT_THAT(outputStream.str(), Eq("<declarationSpecifiers>\n"
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
