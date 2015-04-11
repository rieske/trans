#include "Compiler.h"

#include <cstdlib>
#include <fstream>
#include <iostream>

#include "code_generator/AssemblyGenerator.h"
#include "code_generator/InstructionSet.h"
#include "code_generator/QuadrupleGenerator.h"
#include "code_generator/StackMachine.h"
#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeBuilder.h"
#include "scanner/Scanner.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "CompilerComponentsFactory.h"

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
        parser { this->compilerComponentsFactory->getParser() }
{
}

void Compiler::compile(string sourceFileName) const {
    std::cout << "Compiling " << sourceFileName << "...\n";

    unique_ptr<Scanner> scanner = compilerComponentsFactory->scannerForSourceFile(sourceFileName);
    std::unique_ptr<SyntaxTree> syntaxTree = parser->parse(*scanner, compilerComponentsFactory->newSyntaxTreeBuilder());
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
    codegen::AssemblyGenerator assemblyGenerator {
            std::make_unique<codegen::StackMachine>(&assemblyFile, std::make_unique<codegen::InstructionSet>())
    };
    codegen::QuadrupleGenerator quadrupleGenerator;
    assemblyGenerator.generateAssemblyCode(quadrupleGenerator.generateQuadruplesFrom(*syntaxTree));
    assemblyFile.close();

    if (assemble(assemblyFileName) == 0 && link(sourceFileName) == 0) {
        std::cout << "Successfully compiled and linked\n";
    }
}
