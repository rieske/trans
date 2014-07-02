#include "Compiler.h"

#include <iostream>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../parser/SyntaxTree.h"
#include "../scanner/Scanner.h"
#include "../semantic_analyzer/SemanticAnalyzer.h"

using std::string;
using std::unique_ptr;

Compiler::Compiler(const CompilerComponentsFactory* compilerComponentsFactory) :
		compilerComponentsFactory { compilerComponentsFactory },
		parser { this->compilerComponentsFactory->getParser() } {
}

Compiler::~Compiler() {
}

void Compiler::compile(string sourceFileName) const {
	std::cout << "Compiling " << sourceFileName << "...\n";

	unique_ptr<Scanner> scanner = compilerComponentsFactory->scannerForSourceFile(sourceFileName);
	unique_ptr<SemanticAnalyzer> semanticAnalyzer { compilerComponentsFactory->newSemanticAnalyzer() };
	parser->parse(*scanner, *semanticAnalyzer);
	unique_ptr<SyntaxTree> syntaxTree = semanticAnalyzer->getSyntaxTree();
	// FIXME:
	//if (log) {
		syntaxTree->logXml();
		syntaxTree->printTables();
		syntaxTree->logCode();
	//}
	if (!syntaxTree->hasSemanticErrors()) {
		syntaxTree->setFileName(sourceFileName.c_str());
		std::cout << "Successful semantic analysis\n";
		syntaxTree->outputCode(std::cout);

		CodeGenerator codeGen(sourceFileName.c_str());
		if (0 == codeGen.generateCode(syntaxTree->getCode(), syntaxTree->getSymbolTable())) {
			codeGen.assemble();
			codeGen.link();
		} else {
			std::cerr << "Code generation failed!\n";
		}
	} else {
		std::cerr << "Compilation failed with semantic errors!\n";
	}
}
