#include "ConfigurationParser.h"
#include "HostToolchain.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits.h>
#include <unistd.h>

static const char HELP_OPTION = 'h';
static const char LOGGING_OPTION = 'l';
static const char GRAMMAR_OPTION = 'g';
static const char RESOURCES_BASEDIR_OPTION = 'r';
static const char SCANNER_LOGGING_FLAG = 's';
static const char PARSER_LOGGING_FLAG = 'p';
static const char SYNTAX_TREE_LOGGING_FLAG = 't';
static const char INTERMEDIATE_FORM_LOGGING_FLAG = 'i';

namespace {

bool startsWith(const std::string& s, const char* prefix) {
    return s.rfind(prefix, 0) == 0;
}

bool endsWith(const std::string& s, const char* suffix) {
    const std::size_t n = std::strlen(suffix);
    return s.size() >= n && s.compare(s.size() - n, n, suffix) == 0;
}

bool isObjectOrArchive(const std::string& path) {
    return endsWith(path, ".o") || endsWith(path, ".a") || endsWith(path, ".so")
            || path.find(".so.") != std::string::npos;
}

std::string ensureTrailingSlash(std::string path) {
    if (!path.empty() && path.back() != '/') {
        path.push_back('/');
    }
    return path;
}

// Resolve project root that contains resources/configuration/grammar.bnf.
// Order: TRANS_RESOURCES, walk up from /proc/self/exe, walk up from cwd.
std::string defaultResourcesBasePath() {
    static const char kGrammarMarker[] = "resources/configuration/grammar.bnf";
    if (const char* env = std::getenv("TRANS_RESOURCES")) {
        if (env[0] != '\0') {
            return ensureTrailingSlash(env);
        }
    }

    char exePath[PATH_MAX];
    ssize_t n = ::readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
    if (n > 0) {
        exePath[n] = '\0';
        std::string dir = parentDirectory(exePath);
        if (!dir.empty()) {
            std::string found = findDirWalkingUp(dir, kGrammarMarker, 8, fileExists);
            if (!found.empty()) {
                return found;
            }
            // Common layout: <root>/build/trans -> parent of build is root.
            if (endsWith(dir, "/build")) {
                std::string root = dir.substr(0, dir.size() - 6);
                if (fileExists(root + "/" + kGrammarMarker)) {
                    return ensureTrailingSlash(root);
                }
            }
        }
    }

    char cwd[PATH_MAX];
    if (::getcwd(cwd, sizeof(cwd))) {
        std::string found = findDirWalkingUp(cwd, kGrammarMarker, 8, fileExists);
        if (!found.empty()) {
            return found;
        }
    }
    return {};
}

// Residual gcc noise not claimed by kOptions (table match runs first).
// Single table: exact flags + attached/prefix forms.
bool isIgnoredGccNoise(const std::string& arg) {
    static const char* exact[] = {
        "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-ggdb", "-g3",
        "-pthread", "-pie", "-no-pie", "-shared", "-static",
        "-fPIC", "-fPIE", "-fno-PIE", "-fno-PIC",
        "-pipe", "-s",
        "-MD", "-MMD", "-MP", "-MG",
        "-dM", "-dD",
        nullptr
    };
    for (int i = 0; exact[i]; ++i) {
        if (arg == exact[i]) {
            return true;
        }
    }
    static const char* prefixes[] = {
        "-std=", "-mtune=", "-march=", "-mcpu=",
        "--param",
        "-W", "-f", "-O", "-m",
        nullptr
    };
    for (int i = 0; prefixes[i]; ++i) {
        if (startsWith(arg, prefixes[i])) {
            return true;
        }
    }
    return false;
}

} // namespace

ConfigurationParser::ConfigurationParser(int argc, char **argv) {
	setExecutableName(argv);
	validateArguments(argc, argv);
	parseArgumentsVector(argc, argv);
}

ConfigurationParser::~ConfigurationParser() {
}

Configuration ConfigurationParser::getConfiguration() const {
    return configuration;
}

void ConfigurationParser::parseArgumentsVector(int argc, char **argv) {
	parseOptions(argc, argv);
}

namespace {

// How an option obtains its value (if any).
enum class OptArgForm {
    None,                 // flag only
    SeparateOrAttached,   // -X val or -Xval; value is the path/macro
    SeparateOrAttachedAsIs, // -L: separate → "-L"+next; attached → whole arg
    SeparateOptional,     // -x / -include / -MT: consume next if present
};

enum class OptAction {
    Help,
    CompileOnly,
    PreprocessOnly,
    AssemblyOnly,
    SkipPreprocess,
    OutputFile,
    IncludePath,
    Define,
    Undefine,
    LibPath,
    DepFile,
    Resources,
    Ignore,
};

struct OptionDef {
    const char* exact;   // e.g. "-I" (null if only prefix form)
    const char* prefix;  // attached form e.g. "-I" for -Ipath (null if none)
    OptArgForm form;
    OptAction action;
};

// Table-driven gcc-compatible / trans flags. Special multi-meaning options
// (-l logging vs link, -g grammar vs debug) stay outside the table.
const OptionDef kOptions[] = {
    { "-h", nullptr, OptArgForm::None, OptAction::Help },
    { "--help", nullptr, OptArgForm::None, OptAction::Help },
    { "-c", nullptr, OptArgForm::None, OptAction::CompileOnly },
    { "-E", nullptr, OptArgForm::None, OptAction::PreprocessOnly },
    { "-S", nullptr, OptArgForm::None, OptAction::AssemblyOnly },
    { "-no-preprocess", nullptr, OptArgForm::None, OptAction::SkipPreprocess },
    { "--no-preprocess", nullptr, OptArgForm::None, OptAction::SkipPreprocess },
    { "-o", "-o", OptArgForm::SeparateOrAttached, OptAction::OutputFile },
    { "-I", "-I", OptArgForm::SeparateOrAttached, OptAction::IncludePath },
    { "-D", "-D", OptArgForm::SeparateOrAttached, OptAction::Define },
    { "-U", "-U", OptArgForm::SeparateOrAttached, OptAction::Undefine },
    { "-L", "-L", OptArgForm::SeparateOrAttachedAsIs, OptAction::LibPath },
    { "-r", "-r", OptArgForm::SeparateOrAttached, OptAction::Resources },
    { "-MF", "-MF", OptArgForm::SeparateOrAttached, OptAction::DepFile },
    { "-x", nullptr, OptArgForm::SeparateOptional, OptAction::Ignore },
    { "-include", nullptr, OptArgForm::SeparateOptional, OptAction::Ignore },
    { "-MT", nullptr, OptArgForm::SeparateOptional, OptAction::Ignore },
    { "-MQ", nullptr, OptArgForm::SeparateOptional, OptAction::Ignore },
    { "-g", nullptr, OptArgForm::None, OptAction::Ignore }, // bare -g debug; -gpath handled specially
};

bool isLoggingFlags(const std::string& flags) {
    if (flags.empty()) {
        return false;
    }
    for (char c : flags) {
        if (c != SCANNER_LOGGING_FLAG && c != PARSER_LOGGING_FLAG
                && c != SYNTAX_TREE_LOGGING_FLAG && c != INTERMEDIATE_FORM_LOGGING_FLAG) {
            return false;
        }
    }
    return true;
}

// Match arg against a table entry. Exact forms win over attached prefixes so
// e.g. -include is not parsed as -I + "nclude".
// Sets attachedValue when the attached form matched.
const OptionDef* matchOption(const std::string& arg, std::string& attachedValue) {
    attachedValue.clear();
    for (const OptionDef& def : kOptions) {
        if (def.exact && arg == def.exact) {
            return &def;
        }
    }
    for (const OptionDef& def : kOptions) {
        if (def.prefix && def.form != OptArgForm::None && def.form != OptArgForm::SeparateOptional) {
            const std::size_t n = std::strlen(def.prefix);
            if (arg.size() > n && startsWith(arg, def.prefix)) {
                attachedValue = arg.substr(n);
                return &def;
            }
        }
    }
    return nullptr;
}

} // namespace

void ConfigurationParser::parseOptions(int argc, char **argv) {
    std::vector<std::string> sourceFilePaths;
    std::vector<std::string> objectFilePaths;
    bool resourcesSet = false;

    auto requireNext = [this, argc, argv](int& i, const char* optName) -> std::string {
        if (i + 1 >= argc) {
            outputErrorAndTerminate(std::string("option ") + optName + " requires an argument");
        }
        return argv[++i];
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        // --- special multi-meaning options (not table-driven) ---

        // -Wl,... linker passthrough
        if (startsWith(arg, "-Wl,")) {
            configuration.addLinkArg(arg);
            continue;
        }

        // -l: logging flags (s|p|t|i) vs gcc -l<lib>
        if (startsWith(arg, "-l") && arg.size() > 1 && arg != "-l") {
            std::string flags = arg.substr(2);
            if (isLoggingFlags(flags)) {
                setLogging(flags);
            } else {
                configuration.addLinkArg(arg);
            }
            continue;
        }
        if (arg == "-l") {
            std::string flags = requireNext(i, "-l");
            if (isLoggingFlags(flags)) {
                setLogging(flags);
            } else {
                configuration.addLinkArg(std::string("-l") + flags);
            }
            continue;
        }

        // -g<path.bnf> grammar; -ggdb etc. ignored (bare -g is in the table)
        if (startsWith(arg, "-g") && arg.size() > 1 && arg != "-g") {
            std::string rest = arg.substr(2);
            if (rest.find(".bnf") != std::string::npos || rest.find('/') != std::string::npos) {
                configuration.setGrammarPath(rest);
            }
            continue;
        }

        // --- table-driven options ---
        std::string attached;
        if (const OptionDef* def = matchOption(arg, attached)) {
            std::string value;
            const bool hasAttached = !attached.empty();

            switch (def->form) {
            case OptArgForm::None:
                break;
            case OptArgForm::SeparateOrAttached:
            case OptArgForm::SeparateOrAttachedAsIs:
                if (hasAttached) {
                    value = attached;
                } else {
                    value = requireNext(i, def->exact ? def->exact : def->prefix);
                }
                break;
            case OptArgForm::SeparateOptional:
                if (i + 1 < argc) {
                    ++i;
                }
                break;
            }

            switch (def->action) {
            case OptAction::Help:
                printUsage();
                exit(EXIT_SUCCESS);
            case OptAction::CompileOnly:
                configuration.setCompileOnly(true);
                break;
            case OptAction::PreprocessOnly:
                configuration.setPreprocessOnly(true);
                break;
            case OptAction::AssemblyOnly:
                configuration.setAssemblyOnly(true);
                break;
            case OptAction::SkipPreprocess:
                configuration.setSkipPreprocess(true);
                break;
            case OptAction::OutputFile:
                configuration.setOutputFile(value);
                break;
            case OptAction::IncludePath:
                configuration.addIncludePath(value);
                break;
            case OptAction::Define:
                configuration.addDefine(value);
                break;
            case OptAction::Undefine:
                configuration.addUndefine(value);
                break;
            case OptAction::LibPath:
                if (hasAttached) {
                    configuration.addLinkArg(arg); // whole "-Lpath"
                } else {
                    configuration.addLinkArg(std::string("-L") + value);
                }
                break;
            case OptAction::DepFile:
                configuration.addDepFile(value);
                break;
            case OptAction::Resources:
                configuration.setResourcesBasePath(value);
                resourcesSet = true;
                break;
            case OptAction::Ignore:
                break;
            }
            continue;
        }

        if (isIgnoredGccNoise(arg)) {
            continue;
        }

        if (startsWith(arg, "-")) {
            // Unknown option: keep for link-only (e.g. leftovers like -rdynamic).
            configuration.addLinkArg(arg);
            continue;
        }

        if (isObjectOrArchive(arg)) {
            objectFilePaths.push_back(arg);
        } else {
            sourceFilePaths.push_back(arg);
        }
    }

    // Prefer explicit -r / TRANS_RESOURCES. Otherwise keep the historical
    // relative path when cwd already has resources/; only auto-detect from
    // the binary location when that relative layout is missing (make CC=trans
    // from an out-of-tree build directory such as git/).
    if (!resourcesSet) {
        if (const char* env = std::getenv("TRANS_RESOURCES")) {
            if (env[0] != '\0') {
                configuration.setResourcesBasePath(ensureTrailingSlash(env));
            }
        } else if (!fileExists("resources/configuration/grammar.bnf")) {
            std::string autoBase = defaultResourcesBasePath();
            if (!autoBase.empty()) {
                configuration.setResourcesBasePath(autoBase);
            }
        }
    }

    if (sourceFilePaths.empty()) {
        if (!objectFilePaths.empty() && !configuration.isCompileOnly()
                && !configuration.isPreprocessOnly() && !configuration.isAssemblyOnly()) {
            configuration.setLinkOnly(true);
            configuration.setObjectFiles(objectFilePaths);
            return;
        }
        outputErrorAndTerminate("no source files specified");
    }
    configuration.setSourceFiles(sourceFilePaths);
    if (!objectFilePaths.empty()) {
        configuration.setObjectFiles(objectFilePaths);
    }
}

void ConfigurationParser::setExecutableName(char **argv) {
	if (NULL != argv) {
		executableName = argv[0];
	}
}

void ConfigurationParser::validateArguments(int argc, char **argv) const {
	if (argc <= 1 || argv == NULL) {
		printUsage();
		exit(EXIT_FAILURE);
	}
}

void ConfigurationParser::setLogging(std::string loggingArguments) {
	for (std::string::iterator it = loggingArguments.begin(); it < loggingArguments.end(); it++) {
		switch (*it) {
            case SCANNER_LOGGING_FLAG:
                configuration.enableScannerLogging();
                break;
            case PARSER_LOGGING_FLAG:
                configuration.enableParserLogging();
                break;
            case SYNTAX_TREE_LOGGING_FLAG:
                configuration.enableSyntaxTreeLogging();
                break;
            case INTERMEDIATE_FORM_LOGGING_FLAG:
                configuration.setOutputIntermediateForms();
                break;
            default:
                std::string errorMessage = "Invalid logging flag: ";
                errorMessage += *it;
                outputErrorAndTerminate(errorMessage);
		}
	}
}

void ConfigurationParser::outputErrorAndTerminate(std::string errorMessage) const {
	std::cerr << errorMessage << std::endl;
	std::cerr << std::endl;
	printUsage();
	exit(EXIT_FAILURE);
}

void ConfigurationParser::printUsage() const {
	std::cerr << "Usage: " << std::endl;
	std::cerr << executableName << " [options] source_file..." << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << " -" << HELP_OPTION << "\t\tDisplay this information" << std::endl;
	std::cerr << " -" << LOGGING_OPTION << "<s|p|t|i>\tEnable scanner|parser|syntax tree|intermediate code logging" << std::endl;
	std::cerr << " -" << GRAMMAR_OPTION << "<file_name>\tSpecify custom grammar file" << std::endl;
	std::cerr << " -" << RESOURCES_BASEDIR_OPTION << "<directory_path>\tSpecify custom resources base directory" << std::endl;
	std::cerr << " -I <dir>\t\tAdd include search path (passed to preprocessor)" << std::endl;
	std::cerr << " -D <macro>\t\tDefine macro for preprocessor" << std::endl;
	std::cerr << " -U <macro>\t\tUndefine macro for preprocessor" << std::endl;
	std::cerr << " -E\t\t\tPreprocess only" << std::endl;
	std::cerr << " -S\t\t\tCompile to assembly only" << std::endl;
	std::cerr << " -c\t\t\tCompile to object file only (no link)" << std::endl;
	std::cerr << " -o <file>\t\tPlace output in <file>" << std::endl;
	std::cerr << " --no-preprocess\tSkip the gcc -E preprocessing step" << std::endl;
}
