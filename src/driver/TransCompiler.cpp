#include "TransCompiler.h"

#include <fstream>
#include <iostream>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../semantic_analyzer/syntax_tree.h"

using std::string;

TransCompiler::TransCompiler(Parser& parser) :
		Compiler(),
		parser(parser) {
}

TransCompiler::~TransCompiler() {
}

void TransCompiler::compile(const string fileName) {
	std::ifstream sourceFile(fileName.c_str());
	if (sourceFile.is_open()) {
		doCompile(fileName);
	} else {
		std::cerr << "Error: could not open source file for reading. Filename: " << fileName << std::endl;
	}
}

void TransCompiler::doCompile(const string sourceFileName) {
	std::cout << "Compiling " << sourceFileName << "...\n";

	if (0 == parser.parse(sourceFileName.c_str())) {
		SyntaxTree *tree = parser.getSyntaxTree();
		if (tree != NULL && tree->getErrorFlag() == false) {
			std::cout << "Successful semantic analysis\n";
			tree->outputCode(std::cout);

			CodeGenerator *codeGen = new CodeGenerator(sourceFileName.c_str());
			if (0 == codeGen->generateCode(tree->getCode(), tree->getSymbolTable())) {
				codeGen->assemble();
				codeGen->link();
			} else {
				std::cerr << "Code generation failed!\n";
			}
			delete codeGen;
		} else {
			std::cerr << "Compilation failed with semantic errors!\n";
		}
	} else {
		std::cout << "Parsing failed!\n";
	}
}
