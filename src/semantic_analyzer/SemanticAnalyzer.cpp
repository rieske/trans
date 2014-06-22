#include "SemanticAnalyzer.h"

#include "../scanner/Token.h"
#include "ErrorSyntaxTreeBuilder.h"
#include "SyntaxTree.h"

using std::string;

SemanticAnalyzer::SemanticAnalyzer(ParseTreeBuilder* builder) :
		builder { builder } {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

std::unique_ptr<SyntaxTree> SemanticAnalyzer::getSyntaxTree() {
	return builder->build();
}

void SemanticAnalyzer::withSourceFileName(string fileName) {
	builder->withSourceFileName(fileName);
}

void SemanticAnalyzer::makeTerminalNode(const Token& token) {
	builder->makeTerminalNode(token);
}

void SemanticAnalyzer::makeNonterminalNode(string definingSymbol, Production production) {
	builder->makeNonterminalNode(definingSymbol, production);
}

void SemanticAnalyzer::syntaxError() {
	builder.reset(new ErrorSyntaxTreeBuilder { });
}
