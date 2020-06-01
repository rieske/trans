#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <stack>

#include "parser/AcceptAction.h"
#include "parser/Grammar.h"
#include "parser/ParseTreeBuilder.h"

#include "scanner/Scanner.h"
#include "scanner/Token.h"

namespace {

using namespace parser;
using testing::Eq;
using std::unique_ptr;

class GrammarStub: public Grammar {
    Production production { { "" }, { { "<dummy>" } }, 0 };

public:

    std::size_t ruleCount() const override {
        return 0;
    }

    const Production& getRuleByIndex(int index) const override {
        return production;
    }

    std::vector<Production> getProductionsOfSymbol(const GrammarSymbol& symbol) const override {
        return {};
    }

    std::vector<GrammarSymbol> getTerminals() const override {
        return {};
    }
    std::vector<GrammarSymbol> getNonterminals() const override {
        return {};
    }
};

class ParsingTableStub: public ParsingTable {
public:
    ParsingTableStub() :
            ParsingTable { new GrammarStub { } }
    {
    }
};

class ScannerStub: public Scanner {
public:
    Token nextToken() {
        return {"", "", {"",2}};
    }
};

TEST(AcceptAction, isSerializedAsAcceptWithNoState) {
    AcceptAction acceptAction;

    EXPECT_THAT(acceptAction.serialize(), Eq("a"));
}

TEST(AcceptAction, isDeserializedFromString) {
    ParsingTableStub parsingTable;
    GrammarStub grammar;
    unique_ptr<Action> action { Action::deserialize(std::string { "a" }, parsingTable, grammar) };

    EXPECT_THAT(action->serialize(), Eq("a"));
}

TEST(AcceptAction, acceptsTheParse) {
    AcceptAction acceptAction;
    std::stack<parse_state> parsingStack;
    TokenStream tokenStream { new ScannerStub { } };
    std::unique_ptr<SyntaxTreeBuilder> builder { new ParseTreeBuilder {"test"} };

    bool parsingDone = acceptAction.parse(parsingStack, tokenStream, builder);

    EXPECT_THAT(parsingDone, Eq(true));
}

}
