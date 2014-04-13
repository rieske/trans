#include "TransDriver.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/LR1Parser.h"
#include "../scanner/Scanner.h"
#include "../semantic_analyzer/syntax_tree.h"

using std::string;
using std::vector;

TransDriver::TransDriver(Configuration& configuration, Compiler& compiler) :
		configuration(configuration),
		compiler(compiler) {
}

TransDriver::~TransDriver() {
}

void TransDriver::run() const {

	// FIXME:
	if (configuration.isScannerLoggingEnabled()) {
		Scanner::set_logging("scanner.log");
	}
	if (configuration.isParserLoggingEnabled()) {
		LR1Parser::set_logging("parser.log");
	}

	vector<string> sourceFileNames = configuration.getSourceFileNames();
	vector<string>::const_iterator sourceFileNamesIterator;
	for (sourceFileNamesIterator = sourceFileNames.begin(); sourceFileNamesIterator != sourceFileNames.end(); ++sourceFileNamesIterator) {
		string sourceFileName = *sourceFileNamesIterator;
		compiler.compile(sourceFileName);
	}
}
