#include "Compiler.h"

#include <iostream>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../scanner/Scanner.h"
#include "../semantic_analyzer/SyntaxTree.h"

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

	unique_ptr<Scanner> scanner = compilerComponentsFactory->getScanner(sourceFileName);
	unique_ptr<SyntaxTree> tree = parser->parse(*scanner);
	if (!tree->hasSemanticErrors()) {
		tree->setFileName(sourceFileName.c_str());
		std::cout << "Successful semantic analysis\n";
		tree->outputCode(std::cout);

		CodeGenerator codeGen(sourceFileName.c_str());
		if (0 == codeGen.generateCode(tree->getCode(), tree->getSymbolTable())) {
			codeGen.assemble();
			codeGen.link();
		} else {
			std::cerr << "Code generation failed!\n";
		}
	} else {
		std::cerr << "Compilation failed with semantic errors!\n";
	}
}

