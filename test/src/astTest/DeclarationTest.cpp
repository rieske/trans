#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/Declaration.h"
#include "ast/InitializedDeclarator.h"

#include "semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "ast/DirectDeclarator.h"

#include <sstream>

namespace {

using namespace testing;
using namespace ast;
using namespace semantic_analyzer;

TEST(Declaration, isConstructedUsingDeclarationSpecifiers) {
    DeclarationSpecifiers declSpecs { type::Qualifier::CONST };

    Declaration declaration { declSpecs };
}

}
