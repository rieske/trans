#include "Compiler.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <vector>

#include "CompilerComponentsFactory.h"
#include "LanguageFrontEnd.h"
#include "parser/LR1Parser.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/IrGenerator.h"
#include "codegen/ValueKind.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "symbols/ValueEntry.h"
#include "types/SysVClassify.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/PathWalk.h"
#include "util/Process.h"
#include "util/SourcePath.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

codegen::GlobalVariable toCodegenGlobal(const symbols::ValueEntry& global) {
    codegen::GlobalVariable gv;
    gv.name = global.getName();
    gv.sizeInBytes = global.getType().getSize();
    if (global.isExtern()) {
        gv.emission = codegen::ObjectEmission::Reference;
    } else if (global.isStatic()) {
        gv.emission = codegen::ObjectEmission::DefineInternal;
    } else {
        gv.emission = codegen::ObjectEmission::DefineExternal;
    }
    gv.valueType = codegen::valueKindFromCType(global.getType());
    gv.classification = type::sysv::classify(global.getType());
    if (type::isIntegral(global.getType())) {
        gv.isSigned = type::valueIsSigned(global.getType());
    }
    if (const auto* init = global.globalInitializer()) {
        gv.initializer = *init;
    }
    return gv;
}

// Owns a mkstemps path from construction until destruction. Movable.
class ScopedTempFile {
public:
    explicit ScopedTempFile(const std::string& suffix) {
        const auto dir = std::filesystem::temp_directory_path();
        std::string path = (dir / ("transXXXXXX" + suffix)).string();
        std::vector<char> buffer(path.begin(), path.end());
        buffer.push_back('\0');
        const int fd = ::mkstemps(buffer.data(), static_cast<int>(suffix.size()));
        if (fd < 0) {
            throw std::runtime_error { "unable to create temporary file" };
        }
        ::close(fd);
        path_ = buffer.data();
    }

    ~ScopedTempFile() {
        if (!path_.empty()) {
            std::remove(path_.c_str());
        }
    }

    ScopedTempFile(const ScopedTempFile&) = delete;
    ScopedTempFile& operator=(const ScopedTempFile&) = delete;

    ScopedTempFile(ScopedTempFile&& other) noexcept :
            path_ { std::move(other.path_) } {
        other.path_.clear();
    }

    ScopedTempFile& operator=(ScopedTempFile&& other) noexcept {
        if (this != &other) {
            if (!path_.empty()) {
                std::remove(path_.c_str());
            }
            path_ = std::move(other.path_);
            other.path_.clear();
        }
        return *this;
    }

    const std::string& path() const { return path_; }

private:
    std::string path_;
};

std::string objectPath(const std::string& sourceFileName, bool useOutputPath,
        const std::string& outputPath) {
    if (useOutputPath && !outputPath.empty()) {
        return outputPath;
    }
    return sourceFileName + ".o";
}

struct CompilePlan {
    bool skipPreprocess { false };
    std::optional<std::string> preprocessed;
    std::optional<std::string> assembly;
    std::string objectPath;
};

bool forcesPreprocessor(const Configuration& configuration) {
    return !configuration.getPreprocessorArgs().empty();
}

CompilePlan planCompile(const std::string& sourceFileName, const Configuration& configuration) {
    CompilePlan plan;
    const bool assemblyOnly = configuration.isAssemblyOnly();
    const bool saveTemps = configuration.isSaveTemps();
    const std::string& outputPath = configuration.getOutputPath();

    if (configuration.shouldSkipPreprocess()
            || util::isPreprocessedFile(sourceFileName)
            || !Compiler::sourceFileNeedsGccPreprocessor(sourceFileName, configuration)) {
        plan.skipPreprocess = true;
        plan.preprocessed = sourceFileName;
    } else if (saveTemps) {
        plan.preprocessed = util::withExtension(sourceFileName, ".i");
    }

    if (assemblyOnly && !outputPath.empty()) {
        plan.assembly = outputPath;
    } else if (assemblyOnly || saveTemps) {
        plan.assembly = util::withExtension(sourceFileName, ".s");
    }

    plan.objectPath = objectPath(sourceFileName, configuration.isCompileOnly(), outputPath);
    return plan;
}

std::string materialize(const std::optional<std::string>& path, const std::string& tempSuffix,
        std::optional<ScopedTempFile>& temp) {
    if (path.has_value()) {
        return *path;
    }
    temp.emplace(tempSuffix);
    return temp->path();
}

void assemble(const std::string& assemblyFileName, const std::string& objectFileName,
        AssemblyDialect dialect) {
    switch (dialect) {
    case AssemblyDialect::Intel:
        util::runProcessOrThrow({
                "nasm", "-O1", "-f", "elf64",
                "-o", objectFileName,
                assemblyFileName
        });
        return;
    case AssemblyDialect::AtAndT:
        util::runProcessOrThrow({
                "as", "--64",
                "-o", objectFileName,
                assemblyFileName
        });
        return;
    }
    throw std::logic_error { "unknown AssemblyDialect" };
}

void ensureNonEmptyObjectFile(const std::string& path) {
    if (!util::fileExistsNonEmpty(path)) {
        throw std::runtime_error("empty object " + path);
    }
}

// -E -P: preprocess only, no linemarkers.
// -std=c99 -x c: C99 dialect; accept non-.c paths (functional .src).
// Trailing -D after user flags so product defines win over user overrides.
std::vector<std::string> buildPreprocessArgv(const std::vector<std::string>& sourceFileNames,
        const std::string& outputPath,
        const std::vector<std::string>& preprocessorArgs,
        const std::string& preprocessorStdFlag) {
    std::vector<std::string> argv {
            "gcc", "-E", "-P", "-std=c99", "-x", "c"
    };
    argv.insert(argv.end(), preprocessorArgs.begin(), preprocessorArgs.end());
    argv.push_back("-D__STDC__=0");
    argv.push_back("-DCURL_DISABLE_TYPECHECK");
    argv.push_back("-w");
    if (!preprocessorStdFlag.empty()) {
        argv.push_back("-std=" + preprocessorStdFlag);
    }
    if (!outputPath.empty()) {
        argv.push_back("-o");
        argv.push_back(outputPath);
    }
    argv.insert(argv.end(), sourceFileNames.begin(), sourceFileNames.end());
    return argv;
}

} // namespace

std::vector<std::string> Compiler::linkCommand(const std::vector<std::string>& objectFiles,
        const std::string& executableFileName, const std::vector<std::string>& linkerArgs) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie", "-o", executableFileName };
    argv.insert(argv.end(), objectFiles.begin(), objectFiles.end());
    argv.insert(argv.end(), linkerArgs.begin(), linkerArgs.end());
    return argv;
}

void Compiler::link(const std::vector<std::string>& objectFiles, const std::string& executableFileName,
        const std::vector<std::string>& linkerArgs) {
    util::runProcessOrThrow(linkCommand(objectFiles, executableFileName, linkerArgs));
}

std::string Compiler::defaultExecutablePath(const std::string& sourceFileName) {
    return sourceFileName + ".out";
}

std::vector<std::string> Compiler::preprocessCommand(const std::string& sourceFileName,
        const std::string& outputPath, const Configuration& configuration) {
    return preprocessCommand(std::vector<std::string> { sourceFileName }, outputPath, configuration);
}

std::vector<std::string> Compiler::preprocessCommand(const std::vector<std::string>& sourceFileNames,
        const std::string& outputPath, const Configuration& configuration) {
    return buildPreprocessArgv(sourceFileNames, outputPath, configuration.getPreprocessorArgs(),
            configuration.getPreprocessorStdFlag());
}

bool Compiler::sourceFileNeedsGccPreprocessor(const std::string& sourceFileName,
        const Configuration& configuration) {
    if (forcesPreprocessor(configuration)) {
        return true;
    }
    std::ifstream in { sourceFileName };
    if (!in) {
        return true;
    }
    char ch;
    while (in.get(ch)) {
        if (ch == '#') {
            return true;
        }
    }
    return false;
}

Compiler::Compiler(Configuration configuration) :
        configuration { configuration },
        compilerComponentsFactory { configuration },
        frontEnd { compilerComponentsFactory.makeFrontEnd() },
        parser { std::make_unique<parser::LR1Parser>(frontEnd->table()) }
{
}

std::string Compiler::assembleFile(std::string assemblyFileName, const Configuration& configuration) {
    const std::string objectFileName = objectPath(
            assemblyFileName, configuration.isCompileOnly(), configuration.getOutputPath());
    assemble(assemblyFileName, objectFileName, configuration.getAssemblyDialect());
    ensureNonEmptyObjectFile(objectFileName);
    out << "Successfully assembled " << assemblyFileName << "\n";
    return objectFileName;
}

std::string Compiler::compile(std::string sourceFileName) const {
    out << "Compiling " << sourceFileName << " [" << configuration.assemblyDialectTag() << "]...\n";

    const CompilePlan plan = planCompile(sourceFileName, configuration);

    std::optional<ScopedTempFile> preprocessedTemp;
    const std::string iPath = materialize(plan.preprocessed, ".i", preprocessedTemp);
    if (!plan.skipPreprocess) {
        util::runProcessOrThrow(preprocessCommand(sourceFileName, iPath, configuration));
    }

    // Per-TU lexical state (typedefs, enums). Not process-static.
    scanner::LexicalSession session;
    std::unique_ptr<scanner::Scanner> scanner =
            compilerComponentsFactory.makeScannerForSourceFile(iPath, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder =
            compilerComponentsFactory.makeSyntaxTreeBuilder(&frontEnd->grammar(), session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer { syntaxTreeBuilder->parseExtensions() };
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getDataHomes()) {
        globalVariables.push_back(toCodegenGlobal(global));
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*syntaxTree);

    std::optional<ScopedTempFile> assemblyTemp;
    const std::string sPath = materialize(plan.assembly, ".s", assemblyTemp);
    {
        std::ofstream assemblyFile { sPath };
        if (!assemblyFile) {
            throw std::runtime_error { "Unable to open assembly output file " + sPath };
        }
        std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator =
                compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
        assemblyGenerator->generateAssemblyCode(ir, semanticAnalyzer.getConstants(), globalVariables);
    }

    if (configuration.isAssemblyOnly()) {
        out << "Successfully compiled\n";
        return sPath;
    }

    assemble(sPath, plan.objectPath, configuration.getAssemblyDialect());
    ensureNonEmptyObjectFile(plan.objectPath);
    out << "Successfully compiled\n";
    return plan.objectPath;
}
