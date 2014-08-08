#include "SemanticAnalyzer.h"

#include "AbstractSyntaxTreeNode.h"
#include "SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer() {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::analyze(AbstractSyntaxTreeNode& syntaxTreeTop) {
    SemanticAnalysisVisitor analyzerVisitor;
    syntaxTreeTop.accept(analyzerVisitor);
}

}
