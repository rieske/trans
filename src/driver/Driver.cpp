#include "Driver.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Compiler.h"
#include "ConfigurationParser.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/PathWalk.h"
#include "util/Process.h"
#include "util/SourcePath.h"

#include <limits.h>
#include <unistd.h>

static Logger& err = LogManager::getErrorLogger();
static Logger& out = LogManager::getOutputLogger();

namespace {

struct ClassifiedInput {
    std::string path;
    util::InputKind kind;
};

std::vector<ClassifiedInput> classifyInputs(const std::vector<std::string>& paths) {
    std::vector<ClassifiedInput> inputs;
    inputs.reserve(paths.size());
    for (const auto& path : paths) {
        inputs.push_back({ path, util::classifyInput(path) });
    }
    return inputs;
}

bool anyKind(const std::vector<ClassifiedInput>& inputs, util::InputKind kind) {
    for (const auto& input : inputs) {
        if (input.kind == kind) {
            return true;
        }
    }
    return false;
}

// Single policy matrix for stop-stage vs input kinds and -o rules.
std::optional<std::string> validateInputs(StopAfter stop, const std::vector<ClassifiedInput>& inputs,
        const std::string& outputPath) {
    const bool hasLinkInput = anyKind(inputs, util::InputKind::LinkInput);
    const bool hasAssembly = anyKind(inputs, util::InputKind::Assembly);

    if (hasLinkInput && stop != StopAfter::Link) {
        switch (stop) {
        case StopAfter::Preprocess:
            return std::string { "-E cannot be used with link inputs" };
        case StopAfter::Assembly:
            return std::string { "-S cannot be used with link inputs" };
        case StopAfter::Object:
            return std::string { "-c cannot be used with link inputs" };
        case StopAfter::Link:
            break;
        }
    }

    if (hasAssembly && (stop == StopAfter::Preprocess || stop == StopAfter::Assembly)) {
        if (stop == StopAfter::Preprocess) {
            return std::string { "-E cannot be used with assembly files" };
        }
        return std::string { "-S cannot be used with assembly files" };
    }

    if (!outputPath.empty() && inputs.size() > 1
            && (stop == StopAfter::Object || stop == StopAfter::Assembly)) {
        if (stop == StopAfter::Assembly) {
            return std::string { "cannot specify -o with -S and multiple source files" };
        }
        return std::string { "cannot specify -o with -c and multiple source files" };
    }

    if (stop == StopAfter::Link && outputPath.empty() && (inputs.size() > 1 || hasLinkInput)) {
        return std::string { "linking requires -o" };
    }

    return std::nullopt;
}

// When gcc -E did not write -MF (skip preprocess), create an empty stub so
// `include $(dep)` in makefiles succeeds. Do not overwrite a non-empty file.
bool writeStubDepFiles(const std::vector<std::string>& depFiles) {
    for (const auto& dep : depFiles) {
        if (dep.empty()) {
            continue;
        }
        std::error_code existsEc;
        if (std::filesystem::exists(dep, existsEc) && !existsEc
                && std::filesystem::file_size(dep, existsEc) > 0 && !existsEc) {
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
        std::ofstream outFile(dep, std::ios::app);
        if (!outFile) {
            err << "Failed to write depfile " << dep << "\n";
            return false;
        }
    }
    return true;
}

std::string defaultResourcesBasePath() {
    static const char kGrammarMarker[] = "resources/configuration/grammar.bnf";
    char exePath[PATH_MAX];
    ssize_t n = ::readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (n > 0) {
        exePath[n] = '\0';
        std::string dir = util::parentDirectory(exePath);
        if (!dir.empty()) {
            return util::findDirWalkingUp(dir, kGrammarMarker, 8, util::fileExists);
        }
    }
    return {};
}

void maybeAutoDetectResources(Configuration& configuration) {
    if (util::fileExists(configuration.getGrammarPath())) {
        return;
    }
    std::string autoBase = defaultResourcesBasePath();
    if (!autoBase.empty()) {
        configuration.setResourcesBasePath(autoBase);
    }
}

} // namespace

int Driver::run(int argc, char **argv) const {
    ParseResult parsed = parseCommandLine(argc, argv);
    if (!parsed.configuration) {
        if (parsed.exitCode != 0) {
            err << "Error: " << parsed.message << "\n";
        } else {
            err << parsed.message;
        }
        return parsed.exitCode;
    }

    Configuration configuration = *parsed.configuration;
    if (configuration.isVerbose()) {
        for (const auto& flag : configuration.getIgnoredFlags()) {
            err << "ignoring " << flag << "\n";
        }
    }
    maybeAutoDetectResources(configuration);

    const std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();
    const std::vector<ClassifiedInput> inputs = classifyInputs(sourceFilePaths);
    const StopAfter stop = configuration.stopAfter();
    const std::string outputPath = configuration.getOutputPath();

    if (const auto validationError = validateInputs(stop, inputs, outputPath)) {
        err << "Error: " << *validationError << "\n";
        return 1;
    }

    if (stop == StopAfter::Preprocess) {
        if (inputs.empty()) {
            err << "Error: no source files specified\n";
            return 1;
        }
        auto command = Compiler::preprocessCommand(sourceFilePaths, outputPath, configuration);
        util::ProcessResult result = util::runProcess(command);
        if (result.exitCode != 0) {
            err << "Error: " << result.stderrOutput;
            if (!result.stderrOutput.empty() && result.stderrOutput.back() != '\n') {
                err << "\n";
            }
            return 1;
        }
        if (outputPath.empty()) {
            out << result.stdoutOutput;
        }
        return 0;
    }

    if (configuration.usingCustomGrammar() && inputs.empty()) {
        try {
            Compiler { configuration };
            return 0;
        } catch (const std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            return 1;
        }
    }

    if (inputs.empty()) {
        err << "Error: no source files specified\n";
        return 1;
    }

    int exitCode = 0;
    std::vector<std::string> outputs;
    std::unique_ptr<Compiler> compiler;
    for (const ClassifiedInput& input : inputs) {
        try {
            switch (input.kind) {
            case util::InputKind::LinkInput:
                outputs.push_back(input.path);
                break;
            case util::InputKind::Assembly:
                outputs.push_back(Compiler::assembleFile(input.path, configuration));
                break;
            case util::InputKind::Source:
            case util::InputKind::Preprocessed:
                if (!compiler) {
                    compiler = std::make_unique<Compiler>(configuration);
                }
                outputs.push_back(compiler->compile(input.path));
                break;
            }
        } catch (std::exception& exception) {
            err << "Error: " << exception.what() << "\n";
            exitCode = 1;
        } catch (...) {
            err << "Uncaught exception while compiling " << input.path << "\n";
            exitCode = 1;
        }
    }
    if (exitCode == 0 && !writeStubDepFiles(parsed.depFiles)) {
        exitCode = 1;
    }
    if (exitCode != 0 || configuration.stopsBeforeLink() || outputs.empty()) {
        return exitCode;
    }
    std::string executable = outputPath;
    if (executable.empty()) {
        executable = Compiler::defaultExecutablePath(inputs.front().path);
    }
    try {
        Compiler::link(outputs, executable, configuration.getLinkerArgs());
    } catch (std::exception& exception) {
        err << "Error: " << exception.what() << "\n";
        return 1;
    }
    return 0;
}
