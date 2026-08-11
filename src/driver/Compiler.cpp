#include "Compiler.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/IrGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "types/SysVClassify.h"
#include "types/Type.h"
#include "codegen/ValueKind.h"
#include "types/TypeQuery.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

codegen::ObjectEmission emissionFor(const semantic_analyzer::ValueEntry& symbol) {
    if (symbol.isExtern()) {
        return codegen::ObjectEmission::Reference;
    }
    if (symbol.isStatic()) {
        return codegen::ObjectEmission::DefineInternal;
    }
    return codegen::ObjectEmission::DefineExternal;
}

codegen::GlobalVariable toGlobalVariable(const semantic_analyzer::ValueEntry& symbol) {
    codegen::GlobalVariable gv;
    gv.name = symbol.getName();
    gv.sizeInBytes = symbol.getType().getSize();
    gv.valueType = codegen::valueKindFromCType(symbol.getType());
    gv.classification = type::sysv::classify(symbol.getType());
    gv.initValues = symbol.staticInit();
    gv.emission = emissionFor(symbol);
    return gv;
}

struct UnlinkFile {
    explicit UnlinkFile(std::string path) : path { std::move(path) } {}
    ~UnlinkFile() {
        std::remove(path.c_str());
    }

    UnlinkFile(const UnlinkFile&) = delete;
    UnlinkFile& operator=(const UnlinkFile&) = delete;
    UnlinkFile(UnlinkFile&&) = delete;
    UnlinkFile& operator=(UnlinkFile&&) = delete;

    std::string path;
};

std::string objectPath(const std::string& sourceFileName, bool compileOnly, const std::string& outputPath) {
    if (compileOnly && !outputPath.empty()) {
        return outputPath;
    }
    return sourceFileName + ".o";
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

void Compiler::link(const std::vector<std::string>& objectFiles, const std::string& executableFileName) {
    std::vector<std::string> argv { "gcc", "-m64", "-pie", "-o", executableFileName };
    argv.insert(argv.end(), objectFiles.begin(), objectFiles.end());
    util::runProcessOrThrow(argv);
}

std::string Compiler::defaultExecutablePath(const std::string& sourceFileName) {
    return sourceFileName + ".out";
}

Compiler::Compiler(Configuration configuration) :
        configuration { configuration },
        compilerComponentsFactory { configuration },
        grammar { compilerComponentsFactory.makeGrammar() },
        parser { compilerComponentsFactory.makeParser(&grammar) }
{
}

std::string Compiler::compile(std::string sourceFileName) const {
    out << "Compiling " << sourceFileName << " [" << configuration.assemblyDialectTag() << "]...\n";

    UnlinkFile preprocessed { sourceFileName + ".i" };
    util::runProcessOrThrow({ "gcc", "-E", "-x", "c", "-o", preprocessed.path, sourceFileName });

    // Per-TU lexical state (typedefs, enums). Not process-static.
    scanner::LexicalSession session;
    session.typedefs.add("_Float32", type::floating());
    session.typedefs.add("_Float64", type::doubleFloating());
    session.typedefs.add("_Float128", type::doubleFloating());
    session.typedefs.add("_Float32x", type::floating());
    session.typedefs.add("_Float64x", type::doubleFloating());
    std::unique_ptr<scanner::Scanner> scanner =
            compilerComponentsFactory.makeScannerForSourceFile(preprocessed.path, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder =
            compilerComponentsFactory.makeSyntaxTreeBuilder(&grammar, session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer { syntaxTreeBuilder->parseExtensions() };
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& symbol : semanticAnalyzer.getDataHomes()) {
        globalVariables.push_back(toGlobalVariable(symbol));
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*syntaxTree);

    const std::string assemblyFileName = sourceFileName + ".S";
    const std::string objectFileName = objectPath(
            sourceFileName, configuration.isCompileOnly(), configuration.getOutputPath());

    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "Unable to open assembly output file " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator =
            compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(ir, semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();

    assemble(assemblyFileName, objectFileName, configuration.getAssemblyDialect());
    out << "Successfully compiled\n";
    return objectFileName;
}
