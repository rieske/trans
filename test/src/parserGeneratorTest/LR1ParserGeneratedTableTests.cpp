#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "parser/BNFFileReader.h"
#include "parser/LR1Parser.h"
#include "parser/FilePersistedParsingTable.h"
#include "parser/GeneratedParsingTable.h"
#include "parser/Action.h"
#include "driver/Configuration.h"
#include "driver/CompilerComponentsFactory.h"
#include "parser/SyntaxTreeBuilder.h"
#include "parser/LR1Strategy.h"
#include "parser/LALR1Strategy.h"

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

namespace {

TEST(LR1Parser, parsesTestProgramUsingGeneratedLR1ParsingTable) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/grammars/grammar_original.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("grammars/grammar_original.bnf"));
    LR1Parser parser { new GeneratedParsingTable(&grammar, LR1Strategy{}) };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test", &grammar)));
}

TEST(LR1Parser, parsesTestProgramUsingGeneratedLALR1ParsingTable) {
    Configuration configuration;
    configuration.setResourcesBasePath(getResourcesBaseDir());
    configuration.setGrammarPath("resources/grammars/grammar_original.bnf");

    CompilerComponentsFactory compilerComponentsFactory { configuration };
    //LogManager::registerComponentLogger(Component::PARSER, { &std::cerr });
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("grammars/grammar_original.bnf"));
    LR1Parser parser { new GeneratedParsingTable(&grammar, LALR1Strategy {}) };

    ASSERT_NO_THROW(
            parser.parse(*compilerComponentsFactory.makeScannerForSourceFile(getTestResourcePath("programs/example_prog.src")),
                    compilerComponentsFactory.makeSyntaxTreeBuilder("test", &grammar)));
}

// Sanity check that the generated parsing table is the same as the one checked in.
TEST(LR1Parser, parsingTableIsUnchanged) {
    BNFFileReader reader;
    Grammar grammar = reader.readGrammar(getResourcePath("configuration/grammar.bnf"));
    GeneratedParsingTable parsingTable{&grammar, LALR1Strategy {}};

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

