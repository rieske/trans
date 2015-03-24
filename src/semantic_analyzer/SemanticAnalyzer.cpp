#include "SemanticAnalyzer.h"

#include <iostream>
#include <stdexcept>

#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "parser/ParseTree.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

void SemanticAnalyzer::analyze(parser::SyntaxTree& syntaxTree) {
    syntaxTree.accept(*this);
}

void SemanticAnalyzer::visit(ast::AbstractSyntaxTree& tree) {
    SemanticAnalysisVisitor analyzerVisitor { &std::cerr };
    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

void SemanticAnalyzer::visit(parser::ParseTree& parseTree) {
    throw std::runtime_error { "semantic analysis will not be performed on parse tree" };
}

}

