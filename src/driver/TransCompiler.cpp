#include "TransCompiler.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/code_generator.h"
#include "../parser/Parser.h"
#include "../semantic_analyzer/syntax_tree.h"
#include "../semantic_analyzer/SemanticComponentsFactory.h"
//#include "../semantic_analyzer/SemanticSyntaxTreeBuilder.h"
#include "../semantic_analyzer/SyntaxTreeBuilder.h"
#include "TranslationUnit.h"

using std::string;
using std::unique_ptr;

TransCompiler::TransCompiler(unique_ptr<Parser> parser, unique_ptr<SemanticComponentsFactory> semanticComponentsFactory) :
		parser { std::move(parser) },
		semanticComponentsFactory { std::move(semanticComponentsFactory) } {
}

TransCompiler::~TransCompiler() {
}

void TransCompiler::compile(TranslationUnit& translationUnit) const {
	string fileName = translationUnit.getFileName();
	std::cout << "Compiling " << fileName << "...\n";

	unique_ptr<SyntaxTreeBuilder> syntaxTreeBuilder { semanticComponentsFactory->newSyntaxTreeBuilder() };
	if (0 == parser->parse(translationUnit, *syntaxTreeBuilder)) {
		SyntaxTree *tree = parser->getSyntaxTree();
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

