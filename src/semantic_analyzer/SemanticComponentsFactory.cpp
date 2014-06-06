#include "SemanticComponentsFactory.h"

#include "ParseTreeBuilder.h"
#include "SemanticSyntaxTreeBuilder.h"

SemanticComponentsFactory::SemanticComponentsFactory(bool usingCustomGrammar) :
		usingCustomGrammar { usingCustomGrammar } {
}

SemanticComponentsFactory::~SemanticComponentsFactory() {
}

SyntaxTreeBuilder* SemanticComponentsFactory::newSyntaxTreeBuilder() const {
	SyntaxTreeBuilder* syntaxTreeBuilder;
	if (usingCustomGrammar) {
		syntaxTreeBuilder = new ParseTreeBuilder();
	} else {
		syntaxTreeBuilder = new SemanticSyntaxTreeBuilder();
	}
	return syntaxTreeBuilder;
}
