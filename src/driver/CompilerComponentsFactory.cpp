#include "CompilerComponentsFactory.h"

#include <iostream>

#include "../parser/BNFFileGrammar.h"
#include "../parser/FilePersistedParsingTable.h"
#include "../parser/GeneratedParsingTable.h"
#include "../parser/LR1Parser.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../scanner/LexFileFiniteAutomaton.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SemanticTreeBuilder.h"
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
		configuration(configuration) {
}

CompilerComponentsFactory::~CompilerComponentsFactory() {
}

unique_ptr<Scanner> CompilerComponentsFactory::scannerForSourceFile(std::string sourceFileName,
		std::string scannerConfigurationFileName) const {
	LexFileFiniteAutomaton* automaton { new LexFileFiniteAutomaton(scannerConfigurationFileName) };
	if (configuration.isScannerLoggingEnabled()) {
		Logger logger { &std::cout };
		logger << automaton;
	}
	unique_ptr<Scanner> scanner { new FiniteAutomatonScanner { new TranslationUnit { sourceFileName }, automaton } };
	return scanner;
}

unique_ptr<Parser> CompilerComponentsFactory::getParser() const {
	Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &nullStream };
	LogManager::registerComponentLogger(Component::PARSER, logger);

	ParsingTable* parsingTable;
	if (configuration.usingCustomGrammar()) {
		GeneratedParsingTable* generatedTable = new GeneratedParsingTable(new BNFFileGrammar(configuration.getCustomGrammarFileName()));
		if (configuration.isParserLoggingEnabled()) {
			generatedTable->persistToFile("logs/parsing_table");
		}
		parsingTable = generatedTable;
	} else {
		parsingTable = new FilePersistedParsingTable("resources/configuration/parsing_table",
				new BNFFileGrammar("resources/configuration/grammar.bnf"));
	}

	return unique_ptr<Parser> { new LR1Parser(parsingTable) };
}

unique_ptr<SemanticAnalyzer> CompilerComponentsFactory::newSemanticAnalyzer() const {
	return unique_ptr<SemanticAnalyzer> { (
			configuration.usingCustomGrammar() ? new SemanticAnalyzer { new ParseTreeBuilder() } : new SemanticAnalyzer {
															new semantic_analyzer::SemanticTreeBuilder() }) };
}
