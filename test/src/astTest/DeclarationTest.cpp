#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ast/Declaration.h"
#include "ast/InitializedDeclarator.h"

#include "ast/DirectDeclarator.h"

#include <sstream>

namespace {

using namespace testing;
using namespace ast;

TEST(Declaration, isConstructedUsingDeclarationSpecifiers) {
    DeclarationSpecifiers declSpecs { type::Qualifier::CONST };

    Declaration declaration { declSpecs };
}

}
