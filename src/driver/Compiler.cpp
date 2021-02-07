#include "Compiler.h"

#include <fstream>

#include "CompilerComponentsFactory.h"
#include "codegen/AssemblyGenerator.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "codegen/quadruples/Quadruple.h"

using std::string;
using std::unique_ptr;

using parser::SyntaxTree;
using semantic_analyzer::SemanticAnalyzer;

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

Compiler::Compiler(CompilerComponentsFactory compilerComponentsFactory) :
        compilerComponentsFactory { compilerComponentsFactory },
        parser { this->compilerComponentsFactory.makeParser() }
{
}

void Compiler::compile(string sourceFileName) const {
    std::cout << "Compiling " << sourceFileName << "...\n";

    unique_ptr<Scanner> scanner = compilerComponentsFactory.makeScannerForSourceFile(sourceFileName);
    std::unique_ptr<SyntaxTree> syntaxTree =
        parser->parse(*scanner, compilerComponentsFactory.makeSyntaxTreeBuilder(sourceFileName));

    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    codegen::QuadrupleGenerator quadrupleGenerator;

    // TODO: print intermediate forms only if configured
    semanticAnalyzer.printSymbolTable();
    // TODO: encapsulate quadruples behind intermediate form object
    std::vector<std::unique_ptr<codegen::Quadruple>> quadruples = quadrupleGenerator.generateQuadruplesFrom(*syntaxTree);
    std::cout << "\nquadruples\n";
    for (auto &quadruple : quadruples) {
        std::cout << *quadruple;
    }
    std::cout << "quadruples end\n\n";


    std::string assemblyFileName { sourceFileName + ".S" };
    std::ofstream assemblyFile { assemblyFileName };
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory.makeAssemblyGenerator(&assemblyFile);
    assemblyGenerator->generateAssemblyCode(std::move(quadruples));
    assemblyFile.close();

    if (assemble(assemblyFileName) == 0 && link(sourceFileName) == 0) {
        std::cout << "Successfully compiled and linked\n";
    }
}
