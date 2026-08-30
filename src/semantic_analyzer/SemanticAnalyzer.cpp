#include "SemanticAnalyzer.h"

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

bool SemanticAnalyzer::analyze(ast::AbstractSyntaxTree& tree, const scanner::LexicalSession& session,
        diag::Sink& sink) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    analyzerVisitor.setVlaExpressions(tree.vlaExpressions());
    analyzerVisitor.setSession(&session);
    analyzerVisitor.setSink(&sink);
    analyzerVisitor.installGnuBuiltins();

    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }
    return analyzerVisitor.successfulSemanticAnalysis();
}

} // namespace semantic_analyzer

