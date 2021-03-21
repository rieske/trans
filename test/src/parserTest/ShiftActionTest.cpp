#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stack>
#include <vector>

#include "parser/GrammarBuilder.h"
#include "parser/Action.h"
#include "parser/Grammar.h"
#include "parser/ParseTreeBuilder.h"
#include "parser/SyntaxTree.h"
#include "parser/Production.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "translation_unit/Context.h"

#include "TokenMatcher.h"

namespace {

using namespace parser;

using testing::Eq;

class ParseTreeBuilderMock: public SyntaxTreeBuilder {
public:
    std::unique_ptr<SyntaxTree> build() {
        return {nullptr};
    }

    MOCK_METHOD3(makeTerminalNode, void(std::string type, std::string value, const translation_unit::Context& context));
    MOCK_METHOD1(makeNonterminalNode, void(const Production& production));
};

TEST(ShiftAction, isSerializedAsShiftWithState) {
    ShiftAction shiftAction { 42 };

    ASSERT_THAT(shiftAction.serialize(), Eq("s 42"));
}

TEST(ShiftAction, isDeserializedFromString) {
    GrammarBuilder grammarBuilder;
    grammarBuilder.defineRule("<foo>", {"bar"});
    Grammar grammar = grammarBuilder.build();
    ParsingTable parsingTable {&grammar};

    std::unique_ptr<Action> action { Action::deserialize(std::string { "s 42" }, parsingTable, grammar) };

    ASSERT_THAT(action->serialize(), Eq("s 42"));
}

TEST(ShiftAction, pushesItsStateOnStackAndAdvancesTokenStream) {
    ShiftAction shiftAction { 42 };
    std::stack<parse_state> parsingStack;
    std::vector<scanner::Token> tokens { { "a", "a", { "", 0 } }, { "b", "b", { "", 1 } } };
    int currentToken {0};
    TokenStream tokenStream { [&]() { return tokens[currentToken++]; } };
    ParseTreeBuilderMock parseTreeBuilderMock;

    EXPECT_CALL(parseTreeBuilderMock, makeTerminalNode("a", "a", testing::_));

    bool parsingDone = shiftAction.parse(parsingStack, tokenStream, parseTreeBuilderMock);

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(scanner::Token { "b", "b", { "", 1 } }));
    ASSERT_THAT(parsingStack.top(), Eq(42));
    ASSERT_THAT(parsingDone, Eq(false));
}

}
