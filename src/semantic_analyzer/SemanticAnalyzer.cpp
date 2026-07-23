#include "SemanticAnalyzer.h"

#include <iostream>

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
    return analyzerVisitor.getConstants();
}

std::vector<ValueEntry> SemanticAnalyzer::getGlobalVariables() const {
    return analyzerVisitor.getGlobalVariables();
}

void SemanticAnalyzer::visit(ast::AbstractSyntaxTree& tree) {
    analyzerVisitor.setPendingArrayMembers(&tree.pendingArrayMembers());

    // Phase 1: file-scope symbols only (declarations + function registration).
    // Function bodies are deferred so ARRAY_SIZE member bounds can be re-folded
    // once every file-scope global is in the symbol table.
    analyzerVisitor.setSkipFunctionBodies(true);
    for (const auto& treeNode : tree) {
        treeNode->accept(analyzerVisitor);
    }

    // Single late pass: re-fold struct/union member array bounds (ARRAY_SIZE of
    // file-scope arrays). Mutates shared StructBody layout before any body runs.
    analyzerVisitor.applyPendingArrayMemberBounds();

    // Phase 2: function bodies, with correct aggregate layouts.
    analyzerVisitor.setSkipFunctionBodies(false);
    for (const auto& treeNode : tree) {
        if (auto* function = dynamic_cast<ast::FunctionDefinition*>(treeNode.get())) {
            analyzerVisitor.analyzeFunctionBody(*function);
        }
    }

    analyzerVisitor.setPendingArrayMembers(nullptr);
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
}

void SemanticAnalyzer::visit(parser::ParseTree& parseTree) {
    throw std::runtime_error { "semantic analysis will not be performed on parse tree" };
}

void SemanticAnalyzer::printSymbolTable() const {
    analyzerVisitor.printSymbolTable();
}

} // namespace semantic_analyzer
