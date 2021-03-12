#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stack>
#include <vector>

#include "parser/GrammarBuilder.h"
#include "parser/ShiftAction.h"
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
using std::unique_ptr;

class ScannerStub: public scanner::Scanner {
public:
    scanner::Token nextToken() override {
        return tokens.at(currentToken++);
    }

private:
    std::vector<scanner::Token> tokens { { "a", "a", { "", 0 } }, { "b", "b", { "", 1 } } };
    size_t currentToken { 0 };
};

class ParseTreeBuilderMock: public SyntaxTreeBuilder {
public:
    std::unique_ptr<SyntaxTree> build() {
        return {nullptr};
    }

    MOCK_METHOD3(makeTerminalNode, void(std::string type, std::string value, const translation_unit::Context& context));
    MOCK_METHOD2(makeNonterminalNode, void(int definingSymbol, Production production));
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

    unique_ptr<Action> action { Action::deserialize(std::string { "s 42" }, parsingTable, grammar) };

    ASSERT_THAT(action->serialize(), Eq("s 42"));
}

TEST(ShiftAction, pushesItsStateOnStackAndAdvancesTokenStream) {
    ShiftAction shiftAction { 42 };
    std::stack<parse_state> parsingStack;
    TokenStream tokenStream { new ScannerStub { } };
    ParseTreeBuilderMock* parseTreeBuilderMock { new ParseTreeBuilderMock { } };

    EXPECT_CALL(*parseTreeBuilderMock, makeTerminalNode("a", "a", testing::_));
    std::unique_ptr<SyntaxTreeBuilder> builder { parseTreeBuilderMock };

    bool parsingDone = shiftAction.parse(parsingStack, tokenStream, builder);

    ASSERT_THAT(tokenStream.getCurrentToken(), tokenMatches(scanner::Token { "b", "b", { "", 1 } }));
    ASSERT_THAT(parsingStack.top(), Eq(42));
    ASSERT_THAT(parsingDone, Eq(false));
}

}
