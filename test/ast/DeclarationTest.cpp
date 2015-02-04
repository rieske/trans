#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/Declaration.h"
#include "ast/InitializedDeclarator.h"
#include "ast/types/BaseType.h"

#include "semantic_analyzer/SemanticXmlOutputVisitor.h"

#include <sstream>

namespace {

using namespace testing;
using namespace ast;
using namespace semantic_analyzer;

TEST(Declaration, isConstructedUsingDeclarationSpecifiers) {
    DeclarationSpecifiers declSpecs { TypeQualifier::CONST };

    Declaration declaration { declSpecs };
}

}
