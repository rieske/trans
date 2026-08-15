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

std::vector<ValueEntry> SemanticAnalyzer::getDataHomes() const {
    return analyzerVisitor.getDataHomes();
}

void SemanticAnalyzer::analyze(ast::AbstractSyntaxTree& tree) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    // Sole SA import channel for parse-time enumerators (AST snapshot handoff).
    // Whole-TU before the walk (not C declaration-order scope start).
    for (const auto& entry : tree.parseEnumConstants()) {
        analyzerVisitor.importParseEnumConstant(entry.first, entry.second);
    }
    analyzerVisitor.installGnuBuiltins();

    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

} // namespace semantic_analyzer

