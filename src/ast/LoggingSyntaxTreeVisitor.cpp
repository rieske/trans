#include "LoggingSyntaxTreeVisitor.h"
#include "parser/ParseTreeToSourceConverter.h"
#include "parser/XmlOutputVisitor.h"
#include "AbstractSyntaxTree.h"
#include "semantic_analyzer/SemanticXmlOutputVisitor.h"

#include <fstream>

namespace ast {

LoggingSyntaxTreeVisitor::LoggingSyntaxTreeVisitor(std::string sourceFileName):
    sourceFileName{sourceFileName}
{}

LoggingSyntaxTreeVisitor::~LoggingSyntaxTreeVisitor() = default;

void LoggingSyntaxTreeVisitor::visit(AbstractSyntaxTree& ast) {
    std::ofstream xmlStream { sourceFileName + ".semantic.tree.xml" };
    semantic_analyzer::SemanticXmlOutputVisitor toXml { &xmlStream };
    ast.accept(toXml);
}

void LoggingSyntaxTreeVisitor::visit(parser::ParseTree& parseTree) {
    std::ofstream xmlStream { sourceFileName + ".parse.tree.xml" };
    parser::XmlOutputVisitor toXml { &xmlStream };
    parseTree.accept(toXml);

    std::ofstream parseSourceStream { sourceFileName + ".parse.src" };
    parser::ParseTreeToSourceConverter toSource { &parseSourceStream };
    parseTree.accept(toSource);
}

} // namespace ast

