#include "ErrorSyntaxTreeBuilder.h"

#include <stdexcept>

#include "GrammarSymbol.h"
#include "SyntaxTree.h"

using std::string;

ErrorSyntaxTreeBuilder::ErrorSyntaxTreeBuilder() {
}

ErrorSyntaxTreeBuilder::~ErrorSyntaxTreeBuilder() {
}

SyntaxTree ErrorSyntaxTreeBuilder::build() {
	throw std::runtime_error { "parsing failed with syntax errors" };
}

void ErrorSyntaxTreeBuilder::makeTerminalNode(const Token&) {
}

void ErrorSyntaxTreeBuilder::makeNonterminalNode(string, Production) {
}
