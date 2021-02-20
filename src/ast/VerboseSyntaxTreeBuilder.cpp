#include "VerboseSyntaxTreeBuilder.h"

#include <fstream>

namespace ast {

VerboseSyntaxTreeBuilder::VerboseSyntaxTreeBuilder(std::string sourceFileName):
    parseTreeBuilder {sourceFileName},
    sourceFileName {sourceFileName}
{}

VerboseSyntaxTreeBuilder::~VerboseSyntaxTreeBuilder() = default;

void VerboseSyntaxTreeBuilder::makeTerminalNode(std::string type, std::string value, const translation_unit::Context &context) {
    parseTreeBuilder.makeTerminalNode(type, value, context);
    astBuilder.makeTerminalNode(type, value, context);
}

void VerboseSyntaxTreeBuilder::makeNonterminalNode(std::string definingSymbol, parser::Production production) {
    parseTreeBuilder.makeNonterminalNode(definingSymbol, production);
    astBuilder.makeNonterminalNode(definingSymbol, production);
}

std::unique_ptr<parser::SyntaxTree> VerboseSyntaxTreeBuilder::build() {
    auto parseTree = parseTreeBuilder.build();
    std::ofstream xmlStream { sourceFileName + ".syntax.xml" };
    parseTree->outputXml(xmlStream);
    std::ofstream sourceCodeStream { sourceFileName + ".parse.src" };
    parseTree->outputSource(sourceCodeStream);

    return astBuilder.build();
}

} // namespace ast

