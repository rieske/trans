#include "Compiler.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <vector>

#include "parser/LR1Parser.h"
#include "ast/AbstractSyntaxTree.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/IrGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "symbols/ValueEntry.h"
#include "types/SysVClassify.h"
#include "types/Type.h"
#include "codegen/ValueKind.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"
#include "util/SourcePath.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

codegen::ObjectEmission emissionFor(const symbols::ValueEntry& symbol) {
    if (symbol.isExtern()) {
        return codegen::ObjectEmission::Reference;
    }
    if (symbol.isStatic()) {
        return codegen::ObjectEmission::DefineInternal;
    }
    return codegen::ObjectEmission::DefineExternal;
}

codegen::GlobalVariable toGlobalVariable(const symbols::ValueEntry& symbol) {
    codegen::GlobalVariable gv;
    gv.name = symbol.getName();
    gv.sizeInBytes = symbol.getType().getSize();
    gv.valueType = codegen::valueKindFromCType(symbol.getType());
    gv.classification = type::sysv::classify(symbol.getType());
    gv.initValues = symbol.staticInit();
    gv.emission = emissionFor(symbol);
    return gv;
}

// Owns a mkstemps path from construction until destruction. Movable; no keep flag.
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

// Pure path policy: never creates files.
// nullopt => temporary intermediate, materialized next to ScopedTempFile.
struct CompilePlan {
    bool skipPreprocess { false };
    std::optional<std::string> preprocessed;
    std::optional<std::string> assembly;
    std::string objectPath;
};

bool configurationForcesGccPreprocessor(const Configuration& configuration) {
    return !configuration.getPreprocessorArgs().empty();
}

CompilePlan planCompile(const std::string& sourceFileName, const Configuration& configuration) {
    CompilePlan plan;
    const bool assemblyOnly = configuration.isAssemblyOnly();
    const bool saveTemps = configuration.isSaveTemps();
    const std::string& outputPath = configuration.getOutputPath();

    // Skip gcc -E for .i inputs, or for .c that needs no preprocessor (no '#' / no -I/-D/...).
    if (util::isPreprocessedFile(sourceFileName)
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

// Concrete path is used as-is; nullopt creates and owns a ScopedTempFile.
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
    std::vector<std::string> argv { "gcc", "-E", "-x", "c" };
    const std::string stdFlag = configuration.getPreprocessorStdFlag();
    if (!stdFlag.empty()) {
        argv.push_back("-std=" + stdFlag);
    }
    const auto& preprocessorArgs = configuration.getPreprocessorArgs();
    argv.insert(argv.end(), preprocessorArgs.begin(), preprocessorArgs.end());
    if (!outputPath.empty()) {
        argv.push_back("-o");
        argv.push_back(outputPath);
    }
    argv.insert(argv.end(), sourceFileNames.begin(), sourceFileNames.end());
    return argv;
}

bool Compiler::sourceFileNeedsGccPreprocessor(const std::string& sourceFileName,
        const Configuration& configuration) {
    if (configurationForcesGccPreprocessor(configuration)) {
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
    if (configuration.isVerbose()) {
        out << "Successfully assembled " << assemblyFileName << "\n";
    }
    return objectFileName;
}

std::string Compiler::compile(std::string sourceFileName) const {
    if (configuration.isVerbose()) {
        out << "Compiling " << sourceFileName << " [" << configuration.assemblyDialectTag() << "]...\n";
    }

    const CompilePlan plan = planCompile(sourceFileName, configuration);

    std::optional<ScopedTempFile> preprocessedTemp;
    const std::string iPath = materialize(plan.preprocessed, ".i", preprocessedTemp);
    if (!plan.skipPreprocess) {
        util::runProcessOrThrow(preprocessCommand(sourceFileName, iPath, configuration));
    }

    scanner::LexicalSession session;
    session.typedefs.add("_Float32", type::floating());
    session.typedefs.add("_Float64", type::doubleFloating());
    session.typedefs.add("_Float128", type::doubleFloating());
    session.typedefs.add("_Float32x", type::floating());
    session.typedefs.add("_Float64x", type::doubleFloating());
    std::unique_ptr<scanner::Scanner> scanner =
            compilerComponentsFactory.makeScannerForSourceFile(iPath, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder =
            compilerComponentsFactory.makeSyntaxTreeBuilder(&frontEnd->grammar(), session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);
    auto* tree = dynamic_cast<ast::AbstractSyntaxTree*>(syntaxTree.get());
    if (!tree) {
        throw std::runtime_error { "expected AbstractSyntaxTree" };
    }

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer { configuration.gnuExtensions() };
    semanticAnalyzer.analyze(*tree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& symbol : semanticAnalyzer.getDataHomes()) {
        globalVariables.push_back(toGlobalVariable(symbol));
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*tree);

    // Materialize assembly only after frontend succeeds so failed compiles never create .s temps.
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
        if (configuration.isVerbose()) {
            out << "Successfully compiled\n";
        }
        return sPath;
    }

    assemble(sPath, plan.objectPath, configuration.getAssemblyDialect());
    if (configuration.isVerbose()) {
        out << "Successfully compiled\n";
    }
    return plan.objectPath;
}
