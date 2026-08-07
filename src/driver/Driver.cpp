#include "Driver.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "Compiler.h"
#include "HostToolchain.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& err = LogManager::getErrorLogger();
static Logger& out = LogManager::getOutputLogger();

namespace {

// make includes .d files from -MF; content need not be accurate for pure-trans.
// Create parent dirs and an empty stub so include directives do not fail.
bool writeStubDepFiles(const Configuration& configuration) {
    for (const auto& dep : configuration.getDepFiles()) {
        if (dep.empty()) {
            continue;
        }
        std::filesystem::path path(dep);
        if (path.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);
            if (ec) {
                err << "Failed to create depfile directory for " << dep << ": " << ec.message() << "\n";
                return false;
            }
        }
        std::ofstream outFile(dep, std::ios::trunc);
        if (!outFile) {
            err << "Failed to write depfile " << dep << "\n";
            return false;
        }
    }
    return true;
}

int linkOnly(const Configuration& configuration) {
    // NASM objects use absolute/PC32 relocs; default PIE link fails with
    // R_X86_64_PC32 against libc. Force non-PIE. Always pull in va_tls.o.
    auto argv = buildHostLinkArgv(
            configuration.getObjectFiles(),
            configuration.getOutputFile(),
            configuration.getResourcesBasePath(),
            configuration.getLinkArgs());
    if (argv.empty()) {
        err << "Linking failed: cannot find va_tls.o (build runtime/va_tls.c)\n";
        return 1;
    }
    out << "Linking (host gcc):";
    for (const auto& a : argv) {
        out << " " << a;
    }
    out << "\n";
    try {
        util::runProcessOrThrow(argv);
        return 0;
    } catch (const std::exception& ex) {
        err << "Error: " << ex.what() << "\n";
        return 1;
    }
}

} // namespace

int Driver::run(ConfigurationParser configurationParser) const {
    Configuration configuration = configurationParser.getConfiguration();

    if (configuration.isLinkOnly()) {
        try {
            return linkOnly(configuration);
        } catch (std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            return 1;
        }
    }

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
    // Only emit depfiles after a successful compile (same as former trans-cc).
    if (exitCode == 0 && !writeStubDepFiles(configuration)) {
        exitCode = 1;
    }
    return exitCode;
}
