#include "SemanticAnalyzer.h"

#include <algorithm>
#include <stdexcept>

#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeNode.h"
#include "CodeGeneratingVisitor.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer() {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::analyze(AbstractSyntaxTreeNode& syntaxTreeTop) {
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

std::unique_ptr<SymbolTable> SemanticAnalyzer::getSymbolTable() {
    return std::move(symbolTable);
}

std::vector<Quadruple> SemanticAnalyzer::getQuadrupleCode() const {
    return quadrupleCode;
}

}
