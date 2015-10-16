#include "driver/Compiler.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "codegen/AssemblyGenerator.h"
#include "codegen/QuadrupleGenerator.h"
#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "driver/CompilerComponentsFactory.h"

using std::string;
using std::unique_ptr;

using parser::Parser;
using parser::SyntaxTree;
using semantic_analyzer::SemanticAnalyzer;

int assemble(std::string assemblyFileName) {
    std::string assemblerCommand { "nasm -O1 -f elf64 " + assemblyFileName };
    return system(assemblerCommand.c_str());
}

int link(std::string sourceFileName) {
    //-melf_x86_64 -L/usr/lib64
    // "ld -lc -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o " + sourceFileName + ".out " + sourceFileName + ".o"
    std::string linkerCommand { "gcc -m64 -o " + sourceFileName + ".out " + sourceFileName + ".o" };
    return system(linkerCommand.c_str());
}

Compiler::Compiler(const CompilerComponentsFactory* compilerComponentsFactory) :
        compilerComponentsFactory { compilerComponentsFactory },
        parser { this->compilerComponentsFactory->makeParser() }
{
}

void Compiler::compile(string sourceFileName) const {
    std::cout << "Compiling " << sourceFileName << "...\n";

    unique_ptr<Scanner> scanner = compilerComponentsFactory->makeScannerForSourceFile(sourceFileName);
    std::unique_ptr<SyntaxTree> syntaxTree = parser->parse(*scanner, compilerComponentsFactory->makeSyntaxTreeBuilder());
    // FIXME: move to parser
    //if (log) {
    std::ofstream xmlStream { "logs/syntax_tree.xml" };
    syntaxTree->outputXml(xmlStream);
    std::ofstream sourceCodeStream { "logs/source.c" };
    syntaxTree->outputSource(sourceCodeStream);
    //}

    SemanticAnalyzer semanticAnalyzer;
    semanticAnalyzer.analyze(*syntaxTree);

    std::string assemblyFileName { sourceFileName + ".S" };
    std::ofstream assemblyFile { assemblyFileName };
    std::unique_ptr<codegen::AssemblyGenerator> assemblyGenerator = compilerComponentsFactory->makeAssemblyGenerator(&assemblyFile);
    codegen::QuadrupleGenerator quadrupleGenerator;
    assemblyGenerator->generateAssemblyCode(quadrupleGenerator.generateQuadruplesFrom(*syntaxTree));
    assemblyFile.close();

    if (assemble(assemblyFileName) == 0 && link(sourceFileName) == 0) {
        std::cout << "Successfully compiled and linked\n";
    }
}