#include "Driver.h"

#include <iostream>
#include <vector>

#include "Compiler.h"
#include "CompilerComponentsFactory.h"
#include "util/Logger.h"
#include "util/LogManager.h"

static Logger& err = LogManager::getErrorLogger();

void Driver::run(ConfigurationParser configurationParser) const {

    Configuration configuration = configurationParser.getConfiguration();
	Compiler compiler { configuration };

    std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();
	for (std::string sourceFilePath : sourceFilePaths) {
		try {
			compiler.compile(sourceFilePath);
		} catch (std::exception& exception) {
			err << "Error: " << exception.what() << "\n";
		} catch (...) {
			err << "Uncaught exception while compiling " << sourceFilePath << "\n";
		}
	}
}
