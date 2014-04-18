#include "TransCompiler.h"

#include <iostream>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../semantic_analyzer/syntax_tree.h"
#include "TranslationUnit.h"

using std::string;

TransCompiler::TransCompiler(Parser& parser) :
		Compiler(),
		parser(parser) {
}

TransCompiler::~TransCompiler() {
}

void TransCompiler::compile(TranslationUnit& translationUnit) const {
	string fileName = translationUnit.getFileName();
	std::cout << "Compiling " << fileName << "...\n";

	if (0 == parser.parse(translationUnit)) {
		SyntaxTree *tree = parser.getSyntaxTree();
		if (tree != NULL && tree->getErrorFlag() == false) {
			std::cout << "Successful semantic analysis\n";
			tree->outputCode(std::cout);

			CodeGenerator codeGen(fileName.c_str());
			if (0 == codeGen.generateCode(tree->getCode(), tree->getSymbolTable())) {
				codeGen.assemble();
				codeGen.link();
			} else {
				std::cerr << "Code generation failed!\n";
			}
		} else {
			std::cerr << "Compilation failed with semantic errors!\n";
		}
	} else {
		std::cout << "Parsing failed!\n";
	}
}

