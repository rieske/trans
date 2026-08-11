#include "Driver.h"

#include <memory>
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

    bool anyObject = false;
    for (const auto& path : sourceFilePaths) {
        if (endsWithDotO(path)) {
            anyObject = true;
            break;
        }
    }

    if (configuration.isCompileOnly() && anyObject) {
        err << "Error: -c cannot be used with object files\n";
        return 1;
    }
    if (configuration.isCompileOnly() && !configuration.getOutputPath().empty()
            && sourceFilePaths.size() > 1) {
        err << "Error: cannot specify -o with -c and multiple source files\n";
        return 1;
    }
    if (!configuration.isCompileOnly() && configuration.getOutputPath().empty()
            && (sourceFilePaths.size() > 1 || anyObject)) {
        err << "Error: linking requires -o\n";
        return 1;
    }

    if (configuration.usingCustomGrammar() && sourceFilePaths.empty()) {
        try {
            Compiler { configuration };
            return 0;
        } catch (const std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            return 1;
        }
    }

    int exitCode = 0;
    std::vector<std::string> objectFiles;
    std::unique_ptr<Compiler> compiler;
    for (const std::string& sourceFilePath : sourceFilePaths) {
        if (endsWithDotO(sourceFilePath)) {
            objectFiles.push_back(sourceFilePath);
            continue;
        }
        if (!compiler) {
            compiler = std::make_unique<Compiler>(configuration);
        }
        try {
            objectFiles.push_back(compiler->compile(sourceFilePath));
        } catch (std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            exitCode = 1;
        } catch (...) {
            err << "Uncaught exception while compiling " << sourceFilePath << "\n";
            exitCode = 1;
        }
    }
    if (exitCode != 0 || configuration.isCompileOnly() || objectFiles.empty()) {
        return exitCode;
    }
    std::string executable = configuration.getOutputPath();
    if (executable.empty()) {
        executable = Compiler::defaultExecutablePath(sourceFilePaths.front());
    }
    try {
        Compiler::link(objectFiles, executable);
    } catch (std::exception& exception) {
        err << "Error: " << exception.what() << "\n";
        return 1;
    }
    return 0;
}
