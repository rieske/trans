#include "ConfigurableSemanticComponentsFactory.h"

#include "ParseTreeBuilder.h"
#include "SemanticSyntaxTreeBuilder.h"

ConfigurableSemanticComponentsFactory::ConfigurableSemanticComponentsFactory(bool usingCustomGrammar) :
		usingCustomGrammar { usingCustomGrammar } {
}

ConfigurableSemanticComponentsFactory::~ConfigurableSemanticComponentsFactory() {
}

SyntaxTreeBuilder* ConfigurableSemanticComponentsFactory::newSyntaxTreeBuilder() const {
	SyntaxTreeBuilder* syntaxTreeBuilder;
	if (usingCustomGrammar) {
		syntaxTreeBuilder = new ParseTreeBuilder();
	} else {
		syntaxTreeBuilder = new SemanticSyntaxTreeBuilder();
	}
	return syntaxTreeBuilder;
}
