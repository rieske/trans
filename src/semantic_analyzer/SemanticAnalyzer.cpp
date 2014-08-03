#include "SemanticAnalyzer.h"

#include <iostream>

namespace semantic_analyzer {

SemanticAnalyzer::SemanticAnalyzer() {
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::analyze(const AbstractSyntaxTree& syntaxTree) {
    std::cout << "analyzing\n";
}

}
