#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/LR1Parser.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/CanonicalCollection.h"
#include "parser/FirstTable.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"

#include "ResourceHelpers.h"

#include <fstream>
#include <memory>
#include <sstream>

using namespace testing;
using namespace parser;

namespace {

constexpr const char* kProductGrammar = "configuration/grammar.bnf";

// Generate a parsing table for the product grammar and parse the example program.
void generateAndParseExample(AutomatonKind kind) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath(std::string{"resources/"} + kProductGrammar);

    CompilerComponentsFactory factory { configuration };
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath(kProductGrammar));
    LR1Parser parser { std::make_unique<GeneratedParsingTable>(&grammar, kind) };
    auto syntaxTreeBuilder = factory.makeSyntaxTreeBuilder("test", &grammar);
    ASSERT_NO_THROW(
            parser.parse(*factory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    *syntaxTreeBuilder));
}

// Full generate + parse path for both automaton kinds on the product grammar.
TEST(LR1Parser, generatesAndParsesProductGrammar_LR1) {
    generateAndParseExample(AutomatonKind::LR1);
}

TEST(LR1Parser, generatesAndParsesProductGrammar_LALR1) {
    generateAndParseExample(AutomatonKind::LALR1);
}

// Structural relationship: LR(1) has at least as many states as LALR(1).
TEST(LR1Parser, lr1HasAtLeastAsManyStatesAsLalrOnProductGrammar) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath(kProductGrammar));
    FirstTable first { grammar };
    CanonicalCollection lalr { first, grammar, AutomatonKind::LALR1 };
    CanonicalCollection lr1 { first, grammar, AutomatonKind::LR1 };
    EXPECT_GE(lr1.stateCount(), lalr.stateCount());
    EXPECT_GE(lalr.stateCount(), 1u);
    // Known sizes for the product grammar (guards accidental collapse/explosion).
    EXPECT_EQ(lalr.stateCount(), 399u);
    EXPECT_EQ(lr1.stateCount(), 1747u);
}

// Checked-in product table is LALR; regenerate must match.
TEST(LR1Parser, parsingTableIsUnchanged) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath(kProductGrammar));
    GeneratedParsingTable parsingTable{&grammar, AutomatonKind::LALR1};

    std::string testParsingTableFileName {"test_parsing_table"};
    parsingTable.persistToFile(testParsingTableFileName);

    std::ifstream testParsingTable {testParsingTableFileName};
    std::ifstream realParsingTable {getResourcePath("configuration/parsing_table")};

    if (!std::equal(std::istreambuf_iterator<char>(realParsingTable),
                std::istreambuf_iterator<char>(),
                std::istreambuf_iterator<char>(testParsingTable))) {
        std::stringstream s;
        s << "Parsing table changed!\n";
        s << "If this is expected, regenerate parsing table using ./regenerate-parsing-table.sh ";
        s << "and cp logs/parsing_table resources/configuration/parsing_table";
        throw std::runtime_error {s.str()};
    }
}

} // namespace
