#include "Driver.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Compiler.h"
#include "ConfigurationParser.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"
#include "util/SourcePath.h"

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
// Returns an error message without the "Error: " prefix, or nullopt if valid.
std::optional<std::string> validateInputs(StopAfter stop, const std::vector<ClassifiedInput>& inputs,
        const std::string& outputPath) {
    const bool hasObject = anyKind(inputs, util::InputKind::Object);
    const bool hasAssembly = anyKind(inputs, util::InputKind::Assembly);

    if (hasObject && stop != StopAfter::Link) {
        switch (stop) {
        case StopAfter::Preprocess:
            return std::string { "-E cannot be used with object files" };
        case StopAfter::Assembly:
            return std::string { "-S cannot be used with object files" };
        case StopAfter::Object:
            return std::string { "-c cannot be used with object files" };
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

    if (stop == StopAfter::Link && outputPath.empty() && (inputs.size() > 1 || hasObject)) {
        return std::string { "linking requires -o" };
    }

    return std::nullopt;
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

    const std::vector<std::string> sourceFilePaths = configuration.getSourceFiles();
    const std::vector<ClassifiedInput> inputs = classifyInputs(sourceFilePaths);
    const StopAfter stop = configuration.stopAfter();
    const std::string outputPath = configuration.getOutputPath();

    if (const auto validationError = validateInputs(stop, inputs, outputPath)) {
        err << "Error: " << *validationError << "\n";
        return 1;
    }

    if (stop == StopAfter::Preprocess) {
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

    int exitCode = 0;
    std::vector<std::string> outputs;
    std::unique_ptr<Compiler> compiler;
    for (const ClassifiedInput& input : inputs) {
        try {
            switch (input.kind) {
            case util::InputKind::Object:
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
                if (auto object = compiler->compile(input.path)) {
                    outputs.push_back(*object);
                } else {
                    exitCode = 1;
                }
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
