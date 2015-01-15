#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <sstream>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "ast/types/BaseType.h"
#include "ast/TypeSpecifier.h"
#include "ast/DirectDeclaration.h"
#include "ast/ParameterDeclaration.h"

using testing::Eq;

namespace {

using namespace semantic_analyzer;
using namespace ast;

translation_unit::Context context { "file", 42 };

TEST(SemanticAnalysisVisitor, errsForVoidFunctionArgument) {
    class DeclarationStub: public DirectDeclaration {
    public:

        DeclarationStub() :
                DirectDeclaration { "declarationStub", ::context }
        {
        }

        void accept(ast::AbstractSyntaxTreeVisitor&) override {
        }
    };

    TypeSpecifier type { BaseType::newVoid(), { } };
    DeclarationStub declaration { };
    ParameterDeclaration functionArgument { type, std::make_unique<DeclarationStub>() };

    SemanticAnalysisVisitor visitor;

    visitor.visit(functionArgument);

    EXPECT_THAT(visitor.successfulSemanticAnalysis(), Eq(false));
}

}
