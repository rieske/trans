#include "Compiler.h"

#include <fstream>
#include <stdexcept>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/LexicalSession.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "codegen/quadruples/Quadruple.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

std::string dialectStem(const std::string& sourceFileName, const Configuration& configuration) {
    return sourceFileName + "." + configuration.assemblyDialectTag();
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

    // Per-TU lexical state (typedefs, enums). Not process-static.
    scanner::LexicalSession session;
    std::unique_ptr<scanner::Scanner> scanner =
            compilerComponentsFactory.makeScannerForSourceFile(sourceFileName, session);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder = compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName, &grammar, session);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getGlobalVariables()) {
        codegen::GlobalVariable gv;
        gv.name = global.getName();
        gv.sizeInBytes = global.getType().getSize();
        gv.initializerLiteral = std::to_string(global.getConstantInitializer().value_or(0));
        if (global.getMultiWordInitializer()) {
            gv.multiWordInitializer = *global.getMultiWordInitializer();
        }
        globalVariables.push_back(std::move(gv));
    }

    codegen::QuadrupleGenerator quadrupleGenerator;
    // TODO: encapsulate quadruples behind intermediate form object
    std::vector<std::unique_ptr<codegen::Quadruple>> quadruples = quadrupleGenerator.generateQuadruplesFrom(*syntaxTree);

    if (configuration.isOutputIntermediateForms()) {
        out << "\nsymbol table\n";
        semanticAnalyzer.printSymbolTable();
        out << "symbol table end\n";
        out << "\nquadruples\n";
        for (auto &quadruple : quadruples) {
            out << *quadruple;
        }
        out << "quadruples end\n\n";
    }

    const std::string stem = dialectStem(sourceFileName, configuration);
    const std::string assemblyFileName = stem + ".S";
    const std::string objectFileName = stem + ".o";
    const std::string executableFileName = sourceFileName + ".out";

    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "Unable to open assembly output file " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(std::move(quadruples), semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();

    assemble(assemblyFileName, objectFileName, configuration.getAssemblyDialect());
    link(objectFileName, executableFileName);
    out << "Successfully compiled and linked\n";
}
