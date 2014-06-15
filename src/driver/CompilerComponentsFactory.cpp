#include "CompilerComponentsFactory.h"

#include <iostream>

#include "../parser/BNFReader.h"
#include "../parser/FilePersistedParsingTable.h"
#include "../parser/GeneratedParsingTable.h"
#include "../parser/Grammar.h"
#include "../parser/LR1Parser.h"
#include "../scanner/FiniteAutomatonFactory.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
#include "../util/Logger.h"
#include "../util/LogManager.h"
#include "../util/NullStream.h"
#include "Configuration.h"
#include "TranslationUnit.h"
#include "parser/Action.h"

using std::string;
using std::unique_ptr;

const string grammarConfigurationFileName = "resources/configuration/grammar.bnf";
const string CompilerComponentsFactory::defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";

CompilerComponentsFactory::CompilerComponentsFactory(const Configuration& configuration) :
		configuration(configuration) {
}

CompilerComponentsFactory::~CompilerComponentsFactory() {
}

unique_ptr<Scanner> CompilerComponentsFactory::getScanner(std::string sourceFileName, std::string scannerConfigurationFileName) const {
	FiniteAutomatonFactory* finiteAutomatonFactory { new FiniteAutomatonFactory { scannerConfigurationFileName } };
	if (configuration.isScannerLoggingEnabled()) {
		Logger logger { &std::cout };
		logger << *finiteAutomatonFactory;
	}
	unique_ptr<Scanner> scanner { new FiniteAutomatonScanner { new TranslationUnit { sourceFileName }, finiteAutomatonFactory } };
	return scanner;
}

unique_ptr<Parser> CompilerComponentsFactory::getParser() const {
	// FIXME:
	if (configuration.isParserLoggingEnabled()) {
		LR1Parser::set_logging();
	}
	Logger logger { configuration.isParserLoggingEnabled() ? &std::cout : &nullStream };
	LogManager::registerComponentLogger(Component::PARSER, logger);

	ParsingTable* parsingTable;
	if (configuration.usingCustomGrammar()) {
		const Grammar grammar = BNFReader(configuration.getCustomGrammarFileName()).getGrammar();
		GeneratedParsingTable* generatedTable = new GeneratedParsingTable(grammar);
		if (configuration.isParserLoggingEnabled()) {
			generatedTable->output_table(grammar);
		}
		parsingTable = generatedTable;
	} else {
		parsingTable = new FilePersistedParsingTable("resources/configuration/parsing_table",
				BNFReader("resources/configuration/grammar.bnf").getGrammar());
	}

	return unique_ptr<Parser> { new LR1Parser(parsingTable, new SemanticComponentsFactory { configuration.usingCustomGrammar() }) };

}
