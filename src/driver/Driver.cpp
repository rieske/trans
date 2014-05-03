#include "Driver.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../scanner/Scanner.h"
#include "SourceTranslationUnit.h"

using std::string;
using std::vector;
using std::unique_ptr;

Driver::Driver(const Configuration& configuration, const CompilerComponentsFactory& compilerComponentsFactory) :
		configuration(configuration),
		compilerComponentsFactory(compilerComponentsFactory) {
}

Driver::~Driver() {
}

void Driver::run() const {
	unique_ptr<Compiler> compiler = compilerComponentsFactory.getCompiler();
	unique_ptr<Scanner> scanner = compilerComponentsFactory.getScanner();

	vector<string> sourceFileNames = configuration.getSourceFileNames();
	for (string fileName : sourceFileNames) {
		try {
			SourceTranslationUnit translationUnit { fileName, *scanner };
			compiler->compile(translationUnit);
		} catch (std::runtime_error& exception) {
			std::cerr << exception.what() << std::endl;
		}
	}
}
