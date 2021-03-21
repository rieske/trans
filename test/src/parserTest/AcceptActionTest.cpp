#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stack>

#include "parser/Action.h"
#include "parser/Grammar.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParseTreeBuilder.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace {

using namespace parser;
using testing::Eq;

TEST(AcceptAction, isSerializedAsAcceptWithNoState) {
    AcceptAction acceptAction;

    EXPECT_THAT(acceptAction.serialize(), Eq("a"));
}

TEST(AcceptAction, isDeserializedFromString) {
    GrammarBuilder grammarBuilder;
    grammarBuilder.defineRule("<foo>", {"bar"});
    Grammar grammar = grammarBuilder.build();
    ParsingTable parsingTable {&grammar};
    std::unique_ptr<Action> action { Action::deserialize(std::string { "a" }, parsingTable, grammar) };

    EXPECT_THAT(action->serialize(), Eq("a"));
}

TEST(AcceptAction, acceptsTheParse) {
    AcceptAction acceptAction;
    std::stack<parse_state> parsingStack;
    TokenStream tokenStream { [](){ return scanner::Token{"", "", {"",2}}; }};
    ParseTreeBuilder builder {"test", nullptr};

    bool parsingDone = acceptAction.parse(parsingStack, tokenStream, builder);

    EXPECT_THAT(parsingDone, Eq(true));
}

}
