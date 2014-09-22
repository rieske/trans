#include "SemanticAnalyzer.h"

#include <algorithm>
#include <stdexcept>

#include "code_generator/symbol_table.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "CodeGeneratingVisitor.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer() {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::analyze(ast::AbstractSyntaxTreeNode& syntaxTreeTop) {
    SemanticAnalysisVisitor analyzerVisitor;
    syntaxTreeTop.accept(analyzerVisitor);
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
    symbolTable = analyzerVisitor.getSymbolTable();
    CodeGeneratingVisitor codeGeneratingVisitor;
    syntaxTreeTop.accept(codeGeneratingVisitor);
    quadrupleCode = codeGeneratingVisitor.getQuadruples();
}

std::unique_ptr<code_generator::SymbolTable> SemanticAnalyzer::getSymbolTable() {
    return std::move(symbolTable);
}

std::vector<code_generator::Quadruple> SemanticAnalyzer::getQuadrupleCode() const {
    return quadrupleCode;
}

}
