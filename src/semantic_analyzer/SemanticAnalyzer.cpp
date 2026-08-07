#include "SemanticAnalyzer.h"

#include <stdexcept>

#include "ast/AbstractSyntaxTree.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/FunctionDefinition.h"
#include "parser/ParseTree.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {


SemanticAnalyzer::~SemanticAnalyzer() = default;

void SemanticAnalyzer::analyze(parser::SyntaxTree& syntaxTree) {
    syntaxTree.accept(*this);
}

std::map<std::string, std::string> SemanticAnalyzer::getConstants() const {
    if (!analyzerVisitor) {
        return {};
    }
    return analyzerVisitor->getConstants();
}

std::vector<symbols::ValueEntry> SemanticAnalyzer::getGlobalVariables() const {
    if (!analyzerVisitor) {
        return {};
    }
    return analyzerVisitor->getGlobalVariables();
}

void SemanticAnalyzer::visit(ast::AbstractSyntaxTree& tree) {
    analyzerVisitor = std::make_unique<SemanticAnalysisVisitor>(
            tree.annotations(), tree.pendingArrayMembers());

    // Sole channel for parse-time enumerators into the symbol table (session snapshot).
    for (const auto& entry : tree.parseEnumConstants()) {
        analyzerVisitor->importParseEnumConstant(entry.first, entry.second);
    }

    // Phase 1: file-scope symbols only (declarations + function registration; no bodies).
    for (const auto& treeNode : tree) {
        treeNode->accept(*analyzerVisitor);
    }

    analyzerVisitor->applyPendingArrayMemberBounds();

    // Phase 2: function bodies, with correct aggregate layouts.
    for (const auto& treeNode : tree) {
        if (auto* function = dynamic_cast<ast::FunctionDefinition*>(treeNode.get())) {
            analyzerVisitor->analyzeFunctionBody(*function);
        }
    }

    if (!analyzerVisitor->successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

void SemanticAnalyzer::visit(parser::ParseTree& parseTree) {
    throw std::runtime_error { "semantic analysis will not be performed on parse tree" };
}

void SemanticAnalyzer::printSymbolTable() const {
    if (analyzerVisitor) {
        analyzerVisitor->printSymbolTable();
    }
}

} // namespace semantic_analyzer
