#include "SemanticAnalyzer.h"

#include <stdexcept>

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer(bool gnuExtensions) {
    analyzerVisitor.setGnuExtensions(gnuExtensions);
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

std::map<std::string, std::string> SemanticAnalyzer::getConstants() const {
    return analyzerVisitor.getConstants();
}

std::vector<symbols::ValueEntry> SemanticAnalyzer::getDataHomes() const {
    return analyzerVisitor.getDataHomes();
}

void SemanticAnalyzer::analyze(ast::AbstractSyntaxTree& tree, const scanner::LexicalSession& session) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    analyzerVisitor.setVlaExpressions(tree.vlaExpressions());
    analyzerVisitor.setSession(&session);
    analyzerVisitor.installGnuBuiltins();

    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

} // namespace semantic_analyzer

