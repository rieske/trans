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

std::unique_ptr<SyntaxTree> SemanticAnalyzer::build() {
	return builder->build();
}

void SemanticAnalyzer::withSourceFileName(string fileName) {
	builder->withSourceFileName(fileName);
}

void SemanticAnalyzer::makeTerminalNode(string terminal, Token token) {
	builder->makeTerminalNode(terminal, token);
}

void SemanticAnalyzer::makeNonTerminalNode(string left, int childrenCount, string reduction) {
	builder->makeNonTerminalNode(left, childrenCount, reduction);
}

void SemanticAnalyzer::syntaxError() {
	builder.reset(new ErrorSyntaxTreeBuilder { });
}
