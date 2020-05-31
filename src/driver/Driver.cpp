#include "Driver.h"

#include <iostream>
#include <vector>

#include "Compiler.h"
#include "CompilerComponentsFactory.h"
#include "Configuration.h"

using std::string;
using std::vector;
using std::unique_ptr;

void Driver::run(std::unique_ptr<Configuration> configuration) const {
	vector<string> sourceFileNames = configuration->getSourceFileNames();

	Compiler compiler { std::make_unique<CompilerComponentsFactory>(std::move(configuration)) };
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
