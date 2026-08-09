#include "SemanticAnalyzer.h"

#include <iostream>

#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "parser/ParseTree.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::analyze(parser::SyntaxTree& syntaxTree) {
    syntaxTree.accept(*this);
}

std::map<std::string, std::string> SemanticAnalyzer::getConstants() const {
    return analyzerVisitor.getConstants();
}

std::vector<ValueEntry> SemanticAnalyzer::getFileScopeVariables() const {
    return analyzerVisitor.getFileScopeVariables();
}

void SemanticAnalyzer::visit(ast::AbstractSyntaxTree& tree) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    // Sole SA import channel for parse-time enumerators (AST snapshot handoff).
    // Whole-TU before the walk (not C declaration-order scope start).
    for (const auto& entry : tree.parseEnumConstants()) {
        analyzerVisitor.importParseEnumConstant(entry.first, entry.second);
    }

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

} // namespace semantic_analyzer

