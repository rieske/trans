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

void SemanticAnalyzer::makeNonTerminalNode(string left, int childrenCount, string reduction) {
	builder->makeNonTerminalNode(left, childrenCount, reduction);
}

void SemanticAnalyzer::syntaxError() {
	builder.reset(new ErrorSyntaxTreeBuilder { });
}
