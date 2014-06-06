#include "CompilerComponentsFactory.h"

#include <algorithm>
#include <iostream>

#include "../parser/LR1Parser.h"
#include "../parser/ParsingTable.h"
#include "../scanner/FiniteAutomatonFactory.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../semantic_analyzer/ConfigurableSemanticComponentsFactory.h"
#include "../util/Logger.h"
#include "../util/NullStream.h"
#include "Configuration.h"

using std::string;
using std::unique_ptr;

const string defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";
const string grammarConfigurationFileName = "resources/configuration/grammar.bnf";

CompilerComponentsFactory::CompilerComponentsFactory(const Configuration& configuration) :
		configuration(configuration) {
}

CompilerComponentsFactory::~CompilerComponentsFactory() {
}

unique_ptr<Scanner> CompilerComponentsFactory::getScanner(std::string sourceFileName) const {
	FiniteAutomatonFactory* finiteAutomatonFactory { new FiniteAutomatonFactory(defaultScannerConfigurationFileName) };
	if (configuration.isScannerLoggingEnabled()) {
		Logger logger { std::cout };
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
	Logger logger { configuration.isParserLoggingEnabled() ? std::cout : nullStream };

	ParsingTable* parsingTable;
	if (configuration.usingCustomGrammar()) {
		parsingTable = new ParsingTable(configuration.getCustomGrammarFileName().c_str());
		if (configuration.isParserLoggingEnabled()) {
			parsingTable->log(std::cout);
			parsingTable->output_table();
		}
	} else {
		parsingTable = new ParsingTable();
		if (configuration.isParserLoggingEnabled()) {
			parsingTable->log(std::cout);
		}
	}
	return unique_ptr<Parser> { new LR1Parser(parsingTable, newSemanticComponentsFactory(), logger) };
}

SemanticComponentsFactory* CompilerComponentsFactory::newSemanticComponentsFactory() const {
	return new ConfigurableSemanticComponentsFactory { configuration.usingCustomGrammar() };
}
