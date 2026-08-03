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

void SemanticAnalyzer::analyze(ast::AbstractSyntaxTree& tree) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());

    // Sole channel for parse-time enumerators into the symbol table (session snapshot).
    for (const auto& entry : tree.parseEnumConstants()) {
        analyzerVisitor.importParseEnumConstant(entry.first, entry.second);
    }
    analyzerVisitor.installGnuBuiltins();

    // Single file-scope walk (C 6.2.1). ARRAY_SIZE bounds fold inside
    // visit(Declaration); function bodies run inside visit(FunctionDefinition).
    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }

    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

} // namespace semantic_analyzer
