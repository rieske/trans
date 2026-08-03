#include "SemanticAnalyzer.h"

#include <stdexcept>

#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "parser/ParseTree.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer(const parser::ParseExtensions* extensions) {
    analyzerVisitor.setGnuExtensions(extensions != nullptr);
}

SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::analyze(parser::SyntaxTree& syntaxTree) {
    syntaxTree.accept(*this);
}

std::map<std::string, std::string> SemanticAnalyzer::getConstants() const {
    return analyzerVisitor.getConstants();
}

std::vector<ValueEntry> SemanticAnalyzer::getDataHomes() const {
    return analyzerVisitor.getDataHomes();
}

void SemanticAnalyzer::visit(ast::AbstractSyntaxTree& tree) {
    tree.annotations().clear();
    analyzerVisitor.setAnnotationStore(tree.annotations());
    analyzerVisitor.setPendingArrayMembers(tree.pendingArrayMembers());

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

void SemanticAnalyzer::visit(parser::ParseTree& parseTree) {
    throw std::runtime_error { "semantic analysis will not be performed on parse tree" };
}

} // namespace semantic_analyzer
