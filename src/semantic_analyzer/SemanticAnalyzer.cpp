#include "SemanticAnalyzer.h"

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
    if (analyzerVisitor.successfulSemanticAnalysis()) {
        CodeGeneratingVisitor codeGeneratingVisitor;
        syntaxTreeTop.accept(codeGeneratingVisitor);
    }
}

}
