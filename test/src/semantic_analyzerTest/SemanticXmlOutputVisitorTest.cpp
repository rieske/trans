#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <sstream>

#include "ast/DeclarationSpecifiers.h"
#include "semantic_analyzer/SemanticXmlOutputVisitor.h"
#include "types/Type.h"

using namespace semantic_analyzer;
using testing::HasSubstr;

namespace {

TEST(SemanticXmlOutputVisitor, restrictQualifierIsNamed) {
    ast::DeclarationSpecifiers specs { type::Qualifier::RESTRICT };
    std::ostringstream stream;
    SemanticXmlOutputVisitor visitor { &stream };
    specs.accept(visitor);
    EXPECT_THAT(stream.str(), HasSubstr("<typeQualifier>restrict</typeQualifier>"));
}

}
