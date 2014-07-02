#include "ErrorSyntaxTreeBuilder.h"

#include "../scanner/Token.h"

using std::string;

ErrorSyntaxTreeBuilder::ErrorSyntaxTreeBuilder() {
}

ErrorSyntaxTreeBuilder::~ErrorSyntaxTreeBuilder() {
}

std::unique_ptr<SyntaxTree> ErrorSyntaxTreeBuilder::build() {
	throw std::runtime_error { "parsing failed with syntax errors" };
}

void ErrorSyntaxTreeBuilder::makeTerminalNode(const Token&) {
}

void ErrorSyntaxTreeBuilder::makeNonterminalNode(string, Production) {
}
