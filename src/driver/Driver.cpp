#include "Driver.h"

#include <vector>
#include <stdexcept>

#include "scanner/FiniteAutomatonScanner.h"
#include "parser/LR1Parser.h"
#include "scanner/Scanner.h"
#include "SourceTranslationUnit.h"

using std::string;
using std::vector;

Driver::Driver(const Configuration& configuration, const Compiler& compiler, const CompilerComponentsFactory& compilerComponentsFactory) :
		configuration(configuration),
		compiler(compiler),
		compilerComponentsFactory(compilerComponentsFactory) {
}

Driver::~Driver() {
}

void Driver::run() const {

	// FIXME:
	if (configuration.isParserLoggingEnabled()) {
		LR1Parser::set_logging("parser.log");
	}

	std::unique_ptr<Scanner> scanner = compilerComponentsFactory.getScanner();

	vector<string> sourceFileNames = configuration.getSourceFileNames();
	for (string fileName : sourceFileNames) {
		try {
			SourceTranslationUnit translationUnit { fileName, *scanner.get() };
			compiler.compile(translationUnit);
		} catch (std::invalid_argument& exception) {
			std::cerr << exception.what() << std::endl;
		}
	}
}
