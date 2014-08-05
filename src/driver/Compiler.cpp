#include "Compiler.h"

#include <iostream>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../parser/SyntaxTreeBuilder.h"
#include "../scanner/Scanner.h"
#include "../semantic_analyzer/AbstractSyntaxTree.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "CompilerComponentsFactory.h"

using std::string;
using std::unique_ptr;

using parser::Parser;
using parser::SyntaxTree;
using semantic_analyzer::AbstractSyntaxTree;
using semantic_analyzer::SemanticAnalyzer;

Compiler::Compiler(const CompilerComponentsFactory* compilerComponentsFactory) :
        compilerComponentsFactory { compilerComponentsFactory },
        parser { this->compilerComponentsFactory->getParser() } {
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

    // FIXME:
    //if (log) {
    ((AbstractSyntaxTree*) syntaxTree.get())->printTables();
    ((AbstractSyntaxTree*) syntaxTree.get())->logCode();
    //}
    ((AbstractSyntaxTree*) syntaxTree.get())->outputCode(std::cout);

    CodeGenerator codeGen(sourceFileName.c_str());
    if (0
            == codeGen.generateCode(((AbstractSyntaxTree*) syntaxTree.get())->getCode(),
                    ((AbstractSyntaxTree*) syntaxTree.get())->getSymbolTable())) {
        if (codeGen.assemble() == 0 && codeGen.link() == 0) {
            std::cout << "Successfully compiled and linked\n";
        }
    } else {
        std::cerr << "Code generation failed!\n";
    }
}
