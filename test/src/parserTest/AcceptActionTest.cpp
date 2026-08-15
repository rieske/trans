#include "scanner/LexicalSession.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stack>

#include "parser/Action.h"
#include "parser/Grammar.h"
#include "parser/GrammarBuilder.h"
#include "parser/ParsingTable.h"
#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeBuilder.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace {

using namespace parser;
using testing::Eq;

class NullSyntaxTreeBuilder: public SyntaxTreeBuilder {
public:
    std::unique_ptr<SyntaxTree> build() override { return nullptr; }
    void makeTerminalNode(std::string, std::string, const translation_unit::Context&) override {}
    void makeNonterminalNode(const Production&) override {}
};

TEST(AcceptAction, isSerializedAsAcceptWithNoState) {
    Action acceptAction = Action::accept();

    EXPECT_THAT(acceptAction.serialize(), Eq("a"));
}

TEST(AcceptAction, isDeserializedFromString) {
    GrammarBuilder grammarBuilder;
    grammarBuilder.defineRule("<foo>", {"bar"});
    Grammar grammar = grammarBuilder.build();
    ParsingTable parsingTable {&grammar};
    Action action = Action::deserialize(std::string { "a" }, parsingTable, grammar);

    EXPECT_THAT(action.serialize(), Eq("a"));
}

TEST(AcceptAction, acceptsTheParse) {
    Action acceptAction = Action::accept();
    std::stack<parse_state> parsingStack;
    scanner::LexicalSession session;
    TokenStream tokenStream { [](){ return scanner::Token{"", "", {"",2}}; }, session };
    NullSyntaxTreeBuilder builder;

    bool parsingDone = acceptAction.parse(parsingStack, tokenStream, builder);

    EXPECT_THAT(parsingDone, Eq(true));
}

}
