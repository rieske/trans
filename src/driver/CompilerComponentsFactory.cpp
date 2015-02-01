#include "CompilerComponentsFactory.h"

#include <iostream>

#include "../ast/AbstractSyntaxTreeBuilder.h"
#include "../parser/BNFFileGrammar.h"
#include "../parser/FilePersistedParsingTable.h"
#include "../parser/GeneratedParsingTable.h"
#include "../parser/LALR1Strategy.h"
#include "../parser/LR1Parser.h"
#include "../parser/ParseTreeBuilder.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../scanner/LexFileFiniteAutomaton.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "../util/NullStream.h"
#include "Configuration.h"
#include "TranslationUnit.h"

using std::string;
using std::unique_ptr;

const string grammarConfigurationFileName = "resources/configuration/grammar.bnf";
const string CompilerComponentsFactory::defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";

CompilerComponentsFactory::CompilerComponentsFactory(const Configuration& configuration) :
        configuration { configuration }
{
}

CompilerComponentsFactory::~CompilerComponentsFactory() {
}

unique_ptr<Scanner> CompilerComponentsFactory::scannerForSourceFile(std::string sourceFileName, std::string scannerConfigurationFileName) const {
    LexFileFiniteAutomaton* automaton { new LexFileFiniteAutomaton(scannerConfigurationFileName) };
    if (configuration.isScannerLoggingEnabled()) {
        Logger logger { &std::cout };
        logger << automaton;
    }
    return std::make_unique<FiniteAutomatonScanner>(new TranslationUnit { sourceFileName }, automaton);
}

unique_ptr<parser::Parser> CompilerComponentsFactory::getParser() const {
    Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &nullStream };
    LogManager::registerComponentLogger(Component::PARSER, logger);

    parser::ParsingTable* parsingTable;
    if (configuration.usingCustomGrammar()) {
        parser::GeneratedParsingTable* generatedTable = new parser::GeneratedParsingTable(
                new parser::BNFFileGrammar(configuration.getCustomGrammarFileName()), parser::LALR1Strategy { });
        if (configuration.isParserLoggingEnabled()) {
            generatedTable->persistToFile("logs/parsing_table");
            generatedTable->outputPretty("logs/parsing_table_pretty");
        }
        parsingTable = generatedTable;
    } else {
        parsingTable = new parser::FilePersistedParsingTable("resources/configuration/parsing_table",
                new parser::BNFFileGrammar("resources/configuration/grammar.bnf"));
    }

    return std::make_unique<parser::LR1Parser>(parsingTable);
}

unique_ptr<parser::SyntaxTreeBuilder> CompilerComponentsFactory::newSyntaxTreeBuilder() const {
    //return unique_ptr<parser::SyntaxTreeBuilder> { new parser::ParseTreeBuilder() };
    return configuration.usingCustomGrammar() ?
                                                unique_ptr<parser::SyntaxTreeBuilder> { new parser::ParseTreeBuilder() } :
                                                unique_ptr<parser::SyntaxTreeBuilder> { new ast::AbstractSyntaxTreeBuilder() };
}

unique_ptr<semantic_analyzer::SemanticAnalyzer> CompilerComponentsFactory::newSemanticAnalyzer() const {
    return std::make_unique<semantic_analyzer::SemanticAnalyzer>();
}
