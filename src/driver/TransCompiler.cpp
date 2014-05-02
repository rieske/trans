#include "TransCompiler.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../semantic_analyzer/SyntaxTree.h"
#include "TranslationUnit.h"

using std::string;
using std::unique_ptr;

TransCompiler::TransCompiler(unique_ptr<Parser> parser) :
		parser { std::move(parser) } {
}

TransCompiler::~TransCompiler() {
}

void TransCompiler::compile(TranslationUnit& translationUnit) const {
	string fileName = translationUnit.getFileName();
	std::cout << "Compiling " << fileName << "...\n";

	std::unique_ptr<SyntaxTree> tree = parser->parse(translationUnit);
	if (tree != nullptr && tree->getErrorFlag() == false) {
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
}

