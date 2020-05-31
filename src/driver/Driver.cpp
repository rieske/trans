#include "Driver.h"

#include <iostream>
#include <vector>

#include "Compiler.h"
#include "CompilerComponentsFactory.h"
#include "Configuration.h"

using std::string;
using std::vector;
using std::unique_ptr;

Driver::Driver(unique_ptr<Configuration> configuration) :
		configuration(std::move(configuration)) {
}

Driver::~Driver() {
}

void Driver::run() const {
	Compiler compiler { std::make_unique<CompilerComponentsFactory>(*configuration) };

	vector<string> sourceFileNames = configuration->getSourceFileNames();
	for (string fileName : sourceFileNames) {
		try {
			compiler.compile(fileName);
		} catch (std::exception& exception) {
			std::cerr << exception.what() << std::endl;
		} catch (...) {
			std::cerr << "Uncaught exception while compiling " << fileName << std::endl;
		}
	}
}
