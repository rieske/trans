#include "Compiler.h"

#include <iostream>
#include <vector>
#include <memory>

#include "../ast/AbstractSyntaxTree.h"
#include "../code_generator/code_generator.h"
#include "../code_generator/quadruple.h"
#include "../parser/Parser.h"
#include "../parser/SyntaxTree.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "../scanner/Scanner.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../semantic_analyzer/SymbolTable.h"
#include "CompilerComponentsFactory.h"

#include "code_generator/InstructionSet.h"
#include "code_generator/StackMachine.h"
#include "code_generator/AssemblyGenerator.h"

using std::string;
using std::unique_ptr;

using parser::Parser;
using parser::SyntaxTree;
using ast::AbstractSyntaxTree;
using semantic_analyzer::SemanticAnalyzer;

Compiler::Compiler(const CompilerComponentsFactory* compilerComponentsFactory) :
        compilerComponentsFactory { compilerComponentsFactory },
        parser { this->compilerComponentsFactory->getParser() }
{
}

Compiler::~Compiler() {
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

    unique_ptr<SemanticAnalyzer> semanticAnalyzer { compilerComponentsFactory->newSemanticAnalyzer() };
    syntaxTree->analyzeWith(*semanticAnalyzer);

    auto instructions = std::make_unique<code_generator::InstructionSet>();
    std::ofstream outputFile { sourceFileName + ".S" };
    auto stackMachine = std::make_unique<code_generator::StackMachine>(&outputFile, std::move(instructions));
    code_generator::AssemblyGenerator assemblyGenerator { std::move(stackMachine) };
    //assemblyGenerator.generateAssemblyCode(semanticAnalyzer->getQuadrupleCode());

    std::vector<code_generator::Quadruple_deprecated> quadrupleCode = semanticAnalyzer->getQuadrupleCode();

    code_generator::CodeGenerator codeGen(sourceFileName.c_str());
    if (0 == codeGen.generateCode(quadrupleCode)) {
        if (codeGen.assemble() == 0 && codeGen.link() == 0) {
            std::cout << "Successfully compiled and linked\n";
        }
    } else {
        std::cerr << "Code generation failed!\n";
    }
}
