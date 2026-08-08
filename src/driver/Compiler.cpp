#include "Compiler.h"

#include <fstream>
#include <stdexcept>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/IrGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/ImmediateFormat.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

void preprocess(const std::string& sourceFileName, const std::string& preprocessedFileName) {
    util::runProcessOrThrow({ "gcc", "-E", "-x", "c", "-o", preprocessedFileName, sourceFileName });
}

struct OutputPaths {
    std::string object;
    std::string executable;
};

OutputPaths outputPaths(const std::string& sourceFileName, bool compileOnly, const std::string& outputPath) {
    OutputPaths paths { sourceFileName + ".o", sourceFileName + ".out" };
    if (!outputPath.empty()) {
        if (compileOnly) {
            paths.object = outputPath;
        } else {
            paths.executable = outputPath;
        }
    }
    return paths;
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

void link(const std::string& objectFileName, const std::string& executableFileName) {
    util::runProcessOrThrow({
            "gcc", "-m64", "-pie",
            "-o", executableFileName,
            objectFileName
    });
}

} // namespace

Compiler::Compiler(Configuration configuration) :
        configuration { configuration },
        compilerComponentsFactory { configuration },
        grammar { compilerComponentsFactory.makeGrammar() },
        parser { compilerComponentsFactory.makeParser(&grammar) }
{
}

void Compiler::compile(std::string sourceFileName) const {
    out << "Compiling " << sourceFileName << " [" << configuration.assemblyDialectTag() << "]...\n";

    const std::string preprocessedFileName = sourceFileName + ".i";
    preprocess(sourceFileName, preprocessedFileName);

    // Per-TU lexical state (typedefs, enums). Not process-static.
    scanner::LexicalSession session;
    session.typedefs.add("__builtin_va_list", type::builtinVaListType());
    session.typedefs.add("_Bool", type::unsignedCharacter());
    session.typedefs.add("__int128", type::signedLong());
    session.typedefs.add("_Float32", type::floating());
    session.typedefs.add("_Float64", type::doubleFloating());
    session.typedefs.add("_Float128", type::doubleFloating());
    session.typedefs.add("_Float32x", type::floating());
    session.typedefs.add("_Float64x", type::doubleFloating());
    std::unique_ptr<scanner::Scanner> scanner =
            compilerComponentsFactory.makeScannerForSourceFile(preprocessedFileName, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder =
            compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName, &grammar, session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getGlobalVariables()) {
        codegen::GlobalVariable gv;
        gv.name = global.getName();
        gv.sizeInBytes = global.getType().getSize();
        if (type::isFloating(global.getType())) {
            gv.valueType = codegen::Type::FLOATING;
        }
        if (global.getMultiWordInitializer()) {
            gv.multiWordInitializer = *global.getMultiWordInitializer();
            gv.initializerLiteral = "0";
        } else {
            auto bits = static_cast<unsigned long long>(global.getConstantInitializer().value_or(0));
            gv.initializerLiteral = util::wordImmediate(bits);
        }
        globalVariables.push_back(std::move(gv));
    }

    codegen::IntermediateRepresentation ir = codegen::generateIr(*syntaxTree);

    if (configuration.isOutputIntermediateForms()) {
        out << "\nsymbol table\n";
        semanticAnalyzer.printSymbolTable();
        out << "symbol table end\n";
        out << "\nir\n";
        out << ir;
        out << "ir end\n\n";
    }

    const std::string assemblyFileName = sourceFileName + ".S";
    const OutputPaths paths = outputPaths(
            sourceFileName, configuration.isCompileOnly(), configuration.getOutputPath());
    const std::string& objectFileName = paths.object;
    const std::string& executableFileName = paths.executable;

    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "Unable to open assembly output file " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator =
            compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(ir, semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();

    assemble(assemblyFileName, objectFileName, configuration.getAssemblyDialect());
    if (configuration.isCompileOnly()) {
        out << "Successfully compiled\n";
        return;
    }
    link(objectFileName, executableFileName);
    out << "Successfully compiled and linked\n";
}
