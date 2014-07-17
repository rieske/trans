#include "SemanticAnalyzer.h"

#include "../parser/ErrorSyntaxTreeBuilder.h"
#include "../parser/SyntaxTree.h"

using std::string;

SemanticAnalyzer::SemanticAnalyzer(parser::ParseTreeBuilder* builder) :
		builder { builder } {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

std::unique_ptr<parser::SyntaxTree> SemanticAnalyzer::getSyntaxTree() {
	return builder->build();
}

void SemanticAnalyzer::makeTerminalNode(const Token& token) {
	builder->makeTerminalNode(token);
}

void SemanticAnalyzer::makeNonterminalNode(string definingSymbol, parser::Production production) {
	builder->makeNonterminalNode(definingSymbol, production);
}

void SemanticAnalyzer::syntaxError() {
	builder.reset(new parser::ErrorSyntaxTreeBuilder { });
}
