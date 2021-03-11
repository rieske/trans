#include "ErrorSyntaxTreeBuilder.h"

#include <stdexcept>

namespace parser {

ErrorSyntaxTreeBuilder::ErrorSyntaxTreeBuilder():
    ParseTreeBuilder {""}
{}

ErrorSyntaxTreeBuilder::~ErrorSyntaxTreeBuilder() {
}

std::unique_ptr<SyntaxTree> ErrorSyntaxTreeBuilder::build() {
	throw std::runtime_error { "parsing failed with syntax errors" };
}

void ErrorSyntaxTreeBuilder::makeTerminalNode(std::string, std::string, const translation_unit::Context&) {
}

void ErrorSyntaxTreeBuilder::makeNonterminalNode(int, Production) {
}

} // namespace parser

