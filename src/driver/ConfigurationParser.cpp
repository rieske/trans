#include "ConfigurationParser.h"
#include "ResourcesLocation.h"

#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

enum class OptionId {
    CompileOnly,
    PreprocessOnly,
    AssemblyOnly,
    SaveTemps,
    Output,
    Std,
    Language,
    Masm,
    Resources,
    Log,
    Verbose,
};

enum class ValueForm {
    None,
    SeparateOrEquals,
    StuckOrSeparate,
    // Exact name (separate value) or name + ',' + non-empty remainder (GCC -Wl,opts).
    CommaStuckOrSeparate,
    EqualsOnly,
};

// Assign: last-wins OptionId. Preprocessor: name, plus value unless form is None.
// Linker: None -> name; separate -> name + value; stuck -> original argv token
// (so -lm and -Wl,-as-needed stay single tokens). CommaStuck separate form is
// reconstructed as name + "," + value.
enum class OptionSink {
    Assign,
    Preprocessor,
    Linker,
};

struct OptionSpec {
    std::string_view name;
    ValueForm form;
    OptionSink sink;
    OptionId id {};
};

constexpr OptionSpec assignOpt(std::string_view name, ValueForm form, OptionId id) {
    return OptionSpec { name, form, OptionSink::Assign, id };
}

constexpr OptionSpec preprocessorOpt(std::string_view name, ValueForm form) {
    return OptionSpec { name, form, OptionSink::Preprocessor };
}

constexpr OptionSpec linkerOpt(std::string_view name, ValueForm form) {
    return OptionSpec { name, form, OptionSink::Linker };
}

constexpr OptionSpec kOptions[] = {
        assignOpt("-c", ValueForm::None, OptionId::CompileOnly),
        assignOpt("-E", ValueForm::None, OptionId::PreprocessOnly),
        assignOpt("-S", ValueForm::None, OptionId::AssemblyOnly),
        assignOpt("-save-temps", ValueForm::None, OptionId::SaveTemps),
        assignOpt("-v", ValueForm::None, OptionId::Verbose),
        assignOpt("-o", ValueForm::StuckOrSeparate, OptionId::Output),
        assignOpt("-std", ValueForm::EqualsOnly, OptionId::Std),
        assignOpt("-x", ValueForm::StuckOrSeparate, OptionId::Language),
        assignOpt("-masm", ValueForm::SeparateOrEquals, OptionId::Masm),
        assignOpt("--resources", ValueForm::SeparateOrEquals, OptionId::Resources),
        assignOpt("--log", ValueForm::SeparateOrEquals, OptionId::Log),
        preprocessorOpt("-I", ValueForm::StuckOrSeparate),
        preprocessorOpt("-D", ValueForm::StuckOrSeparate),
        preprocessorOpt("-U", ValueForm::StuckOrSeparate),
        preprocessorOpt("-include", ValueForm::SeparateOrEquals),
        preprocessorOpt("-isystem", ValueForm::SeparateOrEquals),
        preprocessorOpt("-iquote", ValueForm::SeparateOrEquals),
        preprocessorOpt("-MMD", ValueForm::None),
        preprocessorOpt("-MD", ValueForm::None),
        preprocessorOpt("-MP", ValueForm::None),
        preprocessorOpt("-MF", ValueForm::StuckOrSeparate),
        preprocessorOpt("-MQ", ValueForm::StuckOrSeparate),
        preprocessorOpt("-MT", ValueForm::StuckOrSeparate),
        linkerOpt("-l", ValueForm::StuckOrSeparate),
        linkerOpt("-L", ValueForm::StuckOrSeparate),
        linkerOpt("-pthread", ValueForm::None),
        linkerOpt("-Wl", ValueForm::CommaStuckOrSeparate),
};

struct StdInfo {
    const char* name;
    bool gnu;
    bool passToPreprocessor;
};

constexpr StdInfo kStds[] = {
        { "c", false, false },
        { "gnu", true, false },
        { "c89", false, true },
        { "c90", false, true },
        { "c99", false, true },
        { "c11", false, true },
        { "c17", false, true },
        { "c18", false, true },
        { "c23", false, true },
        { "gnu89", true, true },
        { "gnu90", true, true },
        { "gnu99", true, true },
        { "gnu11", true, true },
        { "gnu17", true, true },
        { "gnu18", true, true },
        { "gnu23", true, true },
};

struct Assignment {
    OptionId id;
    std::string value;
};

struct CommandLine {
    std::vector<Assignment> assignments;
    std::vector<std::string> preprocessorArgs;
    std::vector<std::string> linkerArgs;
    std::vector<std::string> ignoredFlags;
    std::vector<std::string> files;
    std::string executable { "trans" };
};

ParseResult errorResult(std::string message) {
    ParseResult result;
    result.exitCode = 1;
    result.message = std::move(message);
    return result;
}

ParseResult helpResult(const std::string& executable) {
    std::ostringstream out;
    out << "Usage:\n";
    out << executable << " [options] file...\n";
    out << "Options:\n";
    out << " -h, --help              Display this information\n";
    out << " -c                      Compile and assemble only (do not link)\n";
    out << " -S                      Compile only; emit assembly\n";
    out << " -E                      Preprocess only\n";
    out << " -save-temps             Keep intermediate .i and .s files\n";
    out << " -v                      Print ignored flags and compile banners\n";
    out << " -O*, -g*, -W*, -f*, -pipe  Accepted and ignored\n";
    out << " -MMD, -MD, -MP, -MF, -MQ, -MT  Write gcc-style header dependencies\n";
    out << " -x c                    Treat the following input as C\n";
    out << " -I <dir>                Add include directory\n";
    out << " -D <macro>              Define preprocessor macro\n";
    out << " -l <lib>                Link with library\n";
    out << " -L <dir>                Add library search path\n";
    out << " -pthread                Link with pthreads\n";
    out << " -Wl,<opts>              Pass comma-separated options to the linker\n";
    out << " -o <file>               Place output in <file>\n";
    out << " -std=<standard>         Language standard (default: gnu)\n";
    out << " -masm=intel|att         Assembly dialect (default: att)\n";
    out << " --resources <dir>       Resources base directory\n";
    out << " --log=scanner,parser    Enable scanner/parser logging\n";
    ParseResult result;
    result.message = out.str();
    return result;
}

bool hasNameEqualsPrefix(std::string_view arg, std::string_view name) {
    return arg.size() > name.size() && arg[name.size()] == '='
            && arg.substr(0, name.size()) == name;
}

bool hasNameCommaPrefix(std::string_view arg, std::string_view name) {
    return arg.size() > name.size() && arg[name.size()] == ','
            && arg.substr(0, name.size()) == name;
}

bool matches(std::string_view arg, const OptionSpec& spec) {
    switch (spec.form) {
    case ValueForm::None:
        return arg == spec.name;
    case ValueForm::EqualsOnly:
    case ValueForm::SeparateOrEquals:
        return arg == spec.name || hasNameEqualsPrefix(arg, spec.name);
    case ValueForm::StuckOrSeparate:
        return arg.size() >= spec.name.size() && arg.substr(0, spec.name.size()) == spec.name;
    case ValueForm::CommaStuckOrSeparate:
        return arg == spec.name || hasNameCommaPrefix(arg, spec.name);
    }
    return false;
}

bool isIgnoredFlag(std::string_view arg) {
    if (arg == "-pipe" || arg == "-pedantic" || arg == "-pedantic-errors") {
        return true;
    }
    if (arg.size() < 2 || arg[0] != '-') {
        return false;
    }
    const char kind = arg[1];
    // -W* is ignored only when findOption did not claim a real option (e.g. -Wl,opts).
    if (kind == 'O' || kind == 'g' || kind == 'f' || kind == 'W') {
        return true;
    }
    return false;
}

const OptionSpec* findOption(std::string_view arg) {
    const OptionSpec* best = nullptr;
    for (const auto& spec : kOptions) {
        if (!matches(arg, spec)) {
            continue;
        }
        if (best == nullptr || spec.name.size() > best->name.size()) {
            best = &spec;
        }
    }
    return best;
}

const StdInfo* findStd(const std::string& name) {
    for (const auto& stdInfo : kStds) {
        if (name == stdInfo.name) {
            return &stdInfo;
        }
    }
    return nullptr;
}

std::string missingArgument(std::string_view option) {
    return "missing argument for " + std::string(option);
}

bool takeNextArg(int& i, int argc, char **argv, std::string_view option, std::string& value,
        std::string& error) {
    if (i + 1 >= argc) {
        error = missingArgument(option);
        return false;
    }
    value = argv[++i];
    return true;
}

bool consumeValue(const OptionSpec& spec, std::string_view arg, int& i, int argc, char **argv,
        std::string& value, std::string& error) {
    switch (spec.form) {
    case ValueForm::None:
        return true;
    case ValueForm::EqualsOnly:
        if (!hasNameEqualsPrefix(arg, spec.name) || arg.size() == spec.name.size() + 1) {
            error = missingArgument(spec.name);
            return false;
        }
        value = std::string(arg.substr(spec.name.size() + 1));
        return true;
    case ValueForm::SeparateOrEquals:
        if (arg == spec.name) {
            return takeNextArg(i, argc, argv, spec.name, value, error);
        }
        value = std::string(arg.substr(spec.name.size() + 1));
        if (value.empty()) {
            error = missingArgument(spec.name);
            return false;
        }
        return true;
    case ValueForm::StuckOrSeparate:
        if (arg == spec.name) {
            return takeNextArg(i, argc, argv, spec.name, value, error);
        }
        value = std::string(arg.substr(spec.name.size()));
        return true;
    case ValueForm::CommaStuckOrSeparate:
        if (arg == spec.name) {
            return takeNextArg(i, argc, argv, spec.name, value, error);
        }
        // arg is name + ',' + remainder; reject empty remainder ("-Wl,").
        if (arg.size() == spec.name.size() + 1) {
            error = missingArgument(spec.name);
            return false;
        }
        value = std::string(arg.substr(spec.name.size() + 1));
        return true;
    }
    error = "unknown option: " + std::string(arg);
    return false;
}

void setAssignment(CommandLine& command, OptionId id, std::string value) {
    for (auto& assignment : command.assignments) {
        if (assignment.id == id) {
            assignment.value = std::move(value);
            return;
        }
    }
    command.assignments.push_back({ id, std::move(value) });
}

void seedFromEnvironment(CommandLine& command) {
    if (const char* resources = std::getenv("TRANS_RESOURCES")) {
        setAssignment(command, OptionId::Resources, resources);
    }
    if (const char* log = std::getenv("TRANS_LOG")) {
        setAssignment(command, OptionId::Log, log);
    }
}

std::optional<ParseResult> walkArgv(int argc, char **argv, CommandLine& command) {
    int i = 1;
    while (i < argc) {
        std::string_view arg { argv[i] };
        if (arg == "--") {
            ++i;
            while (i < argc) {
                command.files.push_back(argv[i]);
                ++i;
            }
            return std::nullopt;
        }
        if (arg == "-h" || arg == "--help") {
            return helpResult(command.executable);
        }
        if (arg.empty() || arg[0] != '-') {
            command.files.push_back(std::string(arg));
            ++i;
            continue;
        }
        const OptionSpec* spec = findOption(arg);
        if (spec == nullptr) {
            if (isIgnoredFlag(arg)) {
                command.ignoredFlags.push_back(std::string(arg));
                ++i;
                continue;
            }
            return errorResult("unknown option: " + std::string(arg));
        }
        std::string value;
        std::string error;
        if (!consumeValue(*spec, arg, i, argc, argv, value, error)) {
            return errorResult(std::move(error));
        }
        if (spec->sink == OptionSink::Preprocessor) {
            command.preprocessorArgs.push_back(std::string(spec->name));
            if (spec->form != ValueForm::None) {
                command.preprocessorArgs.push_back(std::move(value));
            }
        } else if (spec->sink == OptionSink::Linker) {
            if (spec->form == ValueForm::None) {
                command.linkerArgs.push_back(std::string(spec->name));
            } else if (spec->form == ValueForm::CommaStuckOrSeparate) {
                if (arg == spec->name) {
                    command.linkerArgs.push_back(std::string(spec->name) + "," + value);
                } else {
                    command.linkerArgs.push_back(std::string(arg));
                }
            } else if (arg == spec->name) {
                command.linkerArgs.push_back(std::string(spec->name));
                command.linkerArgs.push_back(std::move(value));
            } else {
                command.linkerArgs.push_back(std::string(arg));
            }
        } else {
            setAssignment(command, spec->id, std::move(value));
        }
        ++i;
    }
    return std::nullopt;
}

bool applyLogSpec(Configuration& configuration, const std::string& spec, std::string& error) {
    std::string token;
    auto flush = [&]() {
        if (token.empty()) {
            return true;
        }
        if (token == "s" || token == "scanner") {
            configuration.enableScannerLogging();
        } else if (token == "p" || token == "parser") {
            configuration.enableParserLogging();
        } else {
            error = "invalid log component: " + token + " (expected scanner or parser)";
            return false;
        }
        token.clear();
        return true;
    };
    for (char c : spec) {
        if (c == ',') {
            if (!flush()) {
                return false;
            }
        } else {
            token += c;
        }
    }
    return flush();
}

bool applyStd(Configuration& configuration, const std::string& name, std::string& error) {
    const StdInfo* info = findStd(name);
    if (info == nullptr) {
        error = "invalid language standard: " + name;
        return false;
    }
    configuration.setGnuExtensions(info->gnu);
    configuration.setPreprocessorStdFlag(info->passToPreprocessor ? info->name : "");
    return true;
}

bool applyMasm(Configuration& configuration, const std::string& dialect, std::string& error) {
    if (dialect == "intel") {
        configuration.setAssemblyDialect(AssemblyDialect::Intel);
        return true;
    }
    if (dialect == "att") {
        configuration.setAssemblyDialect(AssemblyDialect::AtAndT);
        return true;
    }
    error = "invalid assembly dialect: " + dialect + " (expected intel or att)";
    return false;
}

bool applyAssignment(Configuration& configuration, const Assignment& assignment, std::string& error) {
    switch (assignment.id) {
    case OptionId::CompileOnly:
        configuration.setCompileOnly();
        return true;
    case OptionId::PreprocessOnly:
        configuration.setPreprocessOnly();
        return true;
    case OptionId::AssemblyOnly:
        configuration.setAssemblyOnly();
        return true;
    case OptionId::SaveTemps:
        configuration.setSaveTemps();
        return true;
    case OptionId::Output:
        configuration.setOutputPath(assignment.value);
        return true;
    case OptionId::Std:
        return applyStd(configuration, assignment.value, error);
    case OptionId::Language:
        if (assignment.value != "c") {
            error = "unsupported language: " + assignment.value + " (only -x c)";
            return false;
        }
        return true;
    case OptionId::Masm:
        return applyMasm(configuration, assignment.value, error);
    case OptionId::Resources:
        configuration.setResourcesBasePath(assignment.value);
        return true;
    case OptionId::Log:
        return applyLogSpec(configuration, assignment.value, error);
    case OptionId::Verbose:
        configuration.setVerbose();
        return true;
    }
    error = "unknown option";
    return false;
}

ParseResult apply(CommandLine command) {
    Configuration configuration;
    for (const auto& assignment : command.assignments) {
        std::string error;
        if (!applyAssignment(configuration, assignment, error)) {
            return errorResult(std::move(error));
        }
    }
    if (!configuration.hasResourcesBasePath()) {
        configuration.setResourcesBasePath(defaultResourcesBase());
    }
    configuration.setPreprocessorArgs(std::move(command.preprocessorArgs));
    configuration.setLinkerArgs(std::move(command.linkerArgs));
    configuration.setIgnoredFlags(std::move(command.ignoredFlags));
    if (command.files.empty()) {
        return errorResult("no input files");
    }
    configuration.setSourceFiles(std::move(command.files));

    ParseResult result;
    result.configuration = std::move(configuration);
    return result;
}

} // namespace

ParseResult parseCommandLine(int argc, char **argv) {
    if (argc <= 0 || argv == nullptr) {
        return errorResult("no input files");
    }

    CommandLine command;
    command.executable = argv[0] ? argv[0] : "trans";
    seedFromEnvironment(command);
    if (auto early = walkArgv(argc, argv, command)) {
        return *early;
    }
    return apply(std::move(command));
}
