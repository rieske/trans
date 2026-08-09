#include "Driver.h"

#include <vector>

#include "Compiler.h"
#include "util/Logger.h"
#include "util/LogManager.h"

static Logger& err = LogManager::getErrorLogger();

namespace {

bool endsWithDotO(const std::string& path) {
    return path.size() >= 2 && path.compare(path.size() - 2, 2, ".o") == 0;
}

} // namespace

int Driver::run(ConfigurationParser configurationParser) const {
    Configuration configuration = configurationParser.getConfiguration();
    std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();

    bool allObjects = !sourceFilePaths.empty();
    bool anyObject = false;
    for (const auto& path : sourceFilePaths) {
        if (endsWithDotO(path)) {
            anyObject = true;
        } else {
            allObjects = false;
        }
    }

    if (allObjects) {
        if (configuration.isCompileOnly()) {
            err << "Error: -c cannot be used with object files\n";
            return 1;
        }
        if (configuration.getOutputPath().empty()) {
            err << "Error: linking object files requires -o\n";
            return 1;
        }
        try {
            Compiler::link(sourceFilePaths, configuration.getOutputPath());
        } catch (std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            return 1;
        }
        return 0;
    }
    if (anyObject) {
        err << "Error: mixing source and object files is not supported\n";
        return 1;
    }
    if (!configuration.getOutputPath().empty() && sourceFilePaths.size() != 1) {
        err << "Error: -o with multiple source files is not supported\n";
        return 1;
    }

    Compiler compiler { configuration };
    int exitCode = 0;
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
