#include "VerboseSyntaxTreeBuilder.h"
#include "ast/LoggingSyntaxTreeVisitor.h"
#include "parser/XmlOutputVisitor.h"

#include <fstream>

namespace ast {

VerboseSyntaxTreeBuilder::VerboseSyntaxTreeBuilder(std::string sourceFileName, const parser::Grammar* grammar):
    astBuilder {grammar},
    parseTreeBuilder {sourceFileName, grammar},
    sourceFileName {sourceFileName},
    loggingVisitor {sourceFileName}
{}

VerboseSyntaxTreeBuilder::~VerboseSyntaxTreeBuilder() = default;

void VerboseSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context &context) {
    parseTreeBuilder.makeTerminalNode(type, value, context);
    astBuilder.makeTerminalNode(type, value, context);
}

void VerboseSyntaxTreeBuilder::makeNonterminalNode(int definingSymbol, parser::Production production) {
    parseTreeBuilder.makeNonterminalNode(definingSymbol, production);
    astBuilder.makeNonterminalNode(definingSymbol, production);
}

std::unique_ptr<parser::SyntaxTree> VerboseSyntaxTreeBuilder::build() {
    auto parseTree = parseTreeBuilder.build();
    parseTree->accept(loggingVisitor);
    auto ast = astBuilder.build();
    ast->accept(loggingVisitor);
    return ast;
}

} // namespace ast

