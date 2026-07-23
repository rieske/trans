#include "Driver.h"

#include <vector>

#include "Compiler.h"
#include "util/Logger.h"
#include "util/LogManager.h"

static Logger& err = LogManager::getErrorLogger();

int Driver::run(ConfigurationParser configurationParser) const {
    Configuration configuration = configurationParser.getConfiguration();
    Compiler compiler { configuration };

    int exitCode = 0;
    std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();
    for (std::string sourceFilePath : sourceFilePaths) {
        try {
            compiler.compile(sourceFilePath);
        } catch (std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            exitCode = 1;
        } catch (...) {
            err << "Uncaught exception while compiling " << sourceFilePath << "\n";
            exitCode = 1;
        }
    }
    return exitCode;
}
