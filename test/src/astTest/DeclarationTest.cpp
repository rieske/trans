#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "src/ast/Declaration.h"
#include "src/ast/InitializedDeclarator.h"

#include "src/semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "src/ast/DirectDeclarator.h"

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
