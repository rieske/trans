#include "TransDriver.h"

#include <iterator>
#include <vector>

#include "parser/LR1Parser.h"
#include "scanner/Scanner.h"
#include "SourceTranslationUnit.h"

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
		SourceTranslationUnit translationUnit(sourceFileName);
		compiler.compile(translationUnit);
	}
}
