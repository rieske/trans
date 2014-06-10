#include "SemanticComponentsFactory.h"

#include "SemanticAnalyzer.h"
#include "SemanticTreeBuilder.h"

SemanticComponentsFactory::SemanticComponentsFactory(bool usingCustomGrammar) :
		usingCustomGrammar { usingCustomGrammar } {
}

SemanticComponentsFactory::~SemanticComponentsFactory() {
}

SemanticAnalyzer* SemanticComponentsFactory::newSemanticAnalyzer() const {
	SemanticAnalyzer* semanticAnalyzer;
	if (usingCustomGrammar) {
		semanticAnalyzer = new SemanticAnalyzer { new ParseTreeBuilder() };
	} else {
		semanticAnalyzer = new SemanticAnalyzer { new SemanticTreeBuilder() };
	}
	return semanticAnalyzer;
}
