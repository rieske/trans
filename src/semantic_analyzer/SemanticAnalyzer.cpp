#include "SemanticAnalyzer.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer(bool gnuExtensions) {
    analyzerVisitor.setGnuExtensions(gnuExtensions);
}


std::map<std::string, std::string> SemanticAnalyzer::getConstants() const {
    return analyzerVisitor.getConstants();
}

std::vector<symbols::ValueEntry> SemanticAnalyzer::getDataHomes() const {
    return analyzerVisitor.getDataHomes();
}

bool SemanticAnalyzer::analyze(ast::AbstractSyntaxTree& tree, diag::Sink& sink) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    analyzerVisitor.setVlaExpressions(tree.vlaExpressions());
    analyzerVisitor.setSink(&sink);
    analyzerVisitor.installGnuBuiltins();

    tree.accept(analyzerVisitor);
    return analyzerVisitor.successfulSemanticAnalysis();
}

} // namespace semantic_analyzer

