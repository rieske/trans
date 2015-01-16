#include "SemanticAnalyzer.h"

#include <iostream>
#include <stdexcept>

#include "ast/AbstractSyntaxTreeNode.h"
#include "CodeGeneratingVisitor.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer() {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::analyze(ast::AbstractSyntaxTreeNode& syntaxTreeTop) {
    SemanticAnalysisVisitor analyzerVisitor { &std::cerr };
    syntaxTreeTop.accept(analyzerVisitor);
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
    CodeGeneratingVisitor codeGeneratingVisitor;
    syntaxTreeTop.accept(codeGeneratingVisitor);
    quadrupleCode = codeGeneratingVisitor.getQuadruples();
}

std::vector<code_generator::Quadruple> SemanticAnalyzer::getQuadrupleCode() const {
    return quadrupleCode;
}

}
