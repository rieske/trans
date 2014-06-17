#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "parser/AcceptAction.h"

using testing::Eq;
using std::unique_ptr;

TEST(AcceptAction, isDescribedAsAcceptWithZeroState) {
	unique_ptr<Action> acceptAction { new AcceptAction() };

	ASSERT_THAT(acceptAction->describe(), Eq("a 0"));
}

/*TEST(AcceptAction, buildsTheSyntaxTree) {
	unique_ptr<Action> acceptAction { new AcceptAction() };

	acceptAction->perform({}, {}, semanticAnalyzer);

	ASSERT_THAT(acceptAction->describe(), Eq("a 0"));
}*/
