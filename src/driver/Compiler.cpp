#include "Compiler.h"

#include <fstream>
#include <stdexcept>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/GlobalVariable.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "codegen/quadruples/Quadruple.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

static Logger& out = LogManager::getOutputLogger();

namespace {

void assemble(const std::string& assemblyFileName) {
    util::runProcessOrThrow({ "nasm", "-O1", "-f", "elf64", assemblyFileName });
}

void link(const std::string& sourceFileName) {
    util::runProcessOrThrow({
            "gcc", "-m64", "-no-pie",
            "-o", sourceFileName + ".out",
            sourceFileName + ".o"
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
    out << "Compiling " << sourceFileName << "...\n";

    std::unique_ptr<scanner::Scanner> scanner = compilerComponentsFactory.makeScannerForSourceFile(sourceFileName);
    std::unique_ptr<parser::SyntaxTreeBuilder> syntaxTreeBuilder = compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName, &grammar);
    std::unique_ptr<parser::SyntaxTree> syntaxTree = parser->parse(*scanner, *syntaxTreeBuilder);

    semantic_analyzer::SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::vector<codegen::GlobalVariable> globalVariables;
    for (const auto& global : semanticAnalyzer.getGlobalVariables()) {
        globalVariables.push_back({
                global.getName(),
                global.getType().getSize(),
                std::to_string(global.getConstantInitializer().value_or(0))
        });
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

    std::string assemblyFileName { sourceFileName + ".S" };
    std::ofstream assemblyFile { assemblyFileName };
    if (!assemblyFile) {
        throw std::runtime_error { "Unable to open assembly output file " + assemblyFileName };
    }
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(std::move(quadruples), semanticAnalyzer.getConstants(), globalVariables);
    assemblyFile.close();

    assemble(assemblyFileName);
    link(sourceFileName);
    out << "Successfully compiled and linked\n";
}
