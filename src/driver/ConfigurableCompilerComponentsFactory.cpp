#include "ConfigurableCompilerComponentsFactory.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "../parser/LR1Parser.h"
#include "../parser/ParsingTable.h"
#include "../scanner/FiniteAutomatonFactory.h"
#include "../scanner/FiniteAutomatonScanner.h"
#include "../semantic_analyzer/ConfigurableSemanticComponentsFactory.h"
#include "../util/Logger.h"
#include "Configuration.h"
#include "TransCompiler.h"

using std::string;
using std::unique_ptr;

const string defaultScannerConfigurationFileName = "resources/configuration/scanner.lex";

ConfigurableCompilerComponentsFactory::ConfigurableCompilerComponentsFactory(const Configuration& configuration) :
		configuration(configuration) {
}

ConfigurableCompilerComponentsFactory::~ConfigurableCompilerComponentsFactory() {
}

unique_ptr<Scanner> ConfigurableCompilerComponentsFactory::getScanner() const {
	unique_ptr<FiniteAutomatonFactory> finiteAutomatonFactory { new FiniteAutomatonFactory(defaultScannerConfigurationFileName) };
	if (configuration.isScannerLoggingEnabled()) {
		Logger logger { };
		logger << *finiteAutomatonFactory;
	}
	unique_ptr<Scanner> scanner { new FiniteAutomatonScanner { std::move(finiteAutomatonFactory) } };
	return scanner;
}

unique_ptr<Compiler> ConfigurableCompilerComponentsFactory::getCompiler() const {
	unique_ptr<Compiler> compiler { new TransCompiler { getParser() } };
	return compiler;
}

unique_ptr<Parser> ConfigurableCompilerComponentsFactory::getParser() const {
	// FIXME:
	if (configuration.isParserLoggingEnabled()) {
		LR1Parser::set_logging("parser.log");
	}

	ParsingTable* parsingTable;
	if (configuration.usingCustomGrammar()) {
		parsingTable = new ParsingTable(configuration.getCustomGrammarFileName().c_str());
		if (configuration.isParserLoggingEnabled()) {
			parsingTable->log(std::cout);
			parsingTable->output_html();
			parsingTable->output_table();
		}
	} else {
		parsingTable = new ParsingTable();
		if (configuration.isParserLoggingEnabled()) {
			parsingTable->log(std::cout);
			parsingTable->output_html();
		}
	}
	return unique_ptr<Parser> { new LR1Parser(parsingTable, newSemanticComponentsFactory()) };
}

SemanticComponentsFactory* ConfigurableCompilerComponentsFactory::newSemanticComponentsFactory() const {
	return new ConfigurableSemanticComponentsFactory { configuration.usingCustomGrammar() };
}
