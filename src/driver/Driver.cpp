#include "Driver.h"

#include <iostream>
#include <vector>

#include "Compiler.h"
#include "CompilerComponentsFactory.h"

void Driver::run(ConfigurationParser configurationParser) const {

    Configuration configuration = configurationParser.getConfiguration();
	Compiler compiler { configuration };

    std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();
	for (std::string sourceFilePath : sourceFilePaths) {
		try {
			compiler.compile(sourceFilePath);
		} catch (std::exception& exception) {
			std::cerr << exception.what() << std::endl;
		} catch (...) {
			std::cerr << "Uncaught exception while compiling " << sourceFilePath << std::endl;
		}
	}
}
