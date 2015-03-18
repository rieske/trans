#include "SemanticAnalyzer.h"

#include <iostream>
#include <stdexcept>

#include "ast/AbstractSyntaxTreeNode.h"
#include "CodeGeneratingVisitor.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

void SemanticAnalyzer::analyze(std::vector<std::unique_ptr<ast::AbstractSyntaxTreeNode> >& translationUnit) {
    SemanticAnalysisVisitor analyzerVisitor { &std::cerr };
    for (const auto& translationElement : translationUnit) {
        translationElement->accept(analyzerVisitor);
    }
    if (!analyzerVisitor.successfulSemanticAnalysis()) {
        throw std::runtime_error { "Semantic errors were detected" };
    }
    CodeGeneratingVisitor codeGeneratingVisitor;
    for (const auto& translationElement : translationUnit) {
        translationElement->accept(codeGeneratingVisitor);
    }
    quadrupleCode = codeGeneratingVisitor.getQuadruples();
}

std::vector<code_generator::Quadruple_deprecated> SemanticAnalyzer::getQuadrupleCode() const {
    return quadrupleCode;
}

}
