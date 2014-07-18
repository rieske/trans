#include "SemanticAnalyzer.h"

#include "../parser/ErrorSyntaxTreeBuilder.h"
#include "../parser/SyntaxTree.h"

using std::string;

SemanticAnalyzer::SemanticAnalyzer(parser::SyntaxTreeBuilder* builder) :
		builder { builder } {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

std::unique_ptr<parser::SyntaxTree> SemanticAnalyzer::getSyntaxTree() {
	return builder->build();
}

void SemanticAnalyzer::makeTerminalNode(std::string type, std::string value, size_t line) {
	builder->makeTerminalNode(type, value, line);
}

void SemanticAnalyzer::makeNonterminalNode(string definingSymbol, parser::Production production) {
	builder->makeNonterminalNode(definingSymbol, production);
}

void SemanticAnalyzer::syntaxError() {
	builder.reset(new parser::ErrorSyntaxTreeBuilder { });
}
