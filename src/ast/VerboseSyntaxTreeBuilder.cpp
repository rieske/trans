#include "VerboseSyntaxTreeBuilder.h"
#include "parser/ParseTreeBuilder.h"

namespace ast {

VerboseSyntaxTreeBuilder::VerboseSyntaxTreeBuilder(std::string sourceFileName):
    parseTreeBuilder {sourceFileName}
{}

void VerboseSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context &context) {
    parseTreeBuilder.makeTerminalNode(type, value, context);
    astBuilder.makeTerminalNode(type, value, context);
}

void VerboseSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
    parseTreeBuilder.makeNonterminalNode(definingSymbol, production);
    astBuilder.makeNonterminalNode(definingSymbol, production);
}

std::unique_ptr<parser::SyntaxTree> VerboseSyntaxTreeBuilder::build() {
    parseTreeBuilder.build();

    return astBuilder.build();
}

} // namespace ast

