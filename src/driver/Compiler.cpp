#include "Compiler.h"

#include <fstream>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "codegen/quadruples/Quadruple.h"
#include "util/Logger.h"
#include "util/LogManager.h"

static Logger& out = LogManager::getOutputLogger();

int assemble(std::string assemblyFileName) {
    std::string assemblerCommand { "nasm -O1 -f elf64 " + assemblyFileName };
    return system(assemblerCommand.c_str());
}

int link(std::string sourceFileName) {
    //-melf_x86_64 -L/usr/lib64
    //std::string linkerCommand { "ld -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2 -e main -o " + sourceFileName + ".out " + sourceFileName + ".o" };
    std::string linkerCommand { "gcc -m64 -no-pie -o " + sourceFileName + ".out " + sourceFileName + ".o" };
    return system(linkerCommand.c_str());
}

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
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(std::move(quadruples), semanticAnalyzer.getConstants());
    assemblyFile.close();

    if (assemble(assemblyFileName) == 0 && link(sourceFileName) == 0) {
        out << "Successfully compiled and linked\n";
    }
}
