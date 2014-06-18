#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "parser/AcceptAction.h"
#include "parser/Grammar.h"

using testing::Eq;
using std::unique_ptr;

TEST(AcceptAction, isDescribedAsAcceptWithNoState) {
	unique_ptr<Action> acceptAction { new AcceptAction() };

	ASSERT_THAT(acceptAction->serialize(), Eq("a"));
}

TEST(AcceptAction, isDeserializedFromString) {
	//Grammar grammar { { }, { } };
	/*unique_ptr<Action> action { Action::deserialize(std::string{"a"}, grammar, nullptr) };

	ASSERT_THAT(action->serialize(), Eq("a"));*/
}

/*TEST(AcceptAction, buildsTheSyntaxTree) {
 unique_ptr<Action> acceptAction { new AcceptAction() };

 acceptAction->perform({}, {}, semanticAnalyzer);

 ASSERT_THAT(acceptAction->describe(), Eq("a 0"));
 }*/
