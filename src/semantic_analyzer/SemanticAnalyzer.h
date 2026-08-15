#ifndef SEMANTICANALYZER_H_
#define SEMANTICANALYZER_H_

#include <map>
#include <vector>

#include "ast/AbstractSyntaxTree.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(bool gnuExtensions = true);
    ~SemanticAnalyzer();

    void analyze(ast::AbstractSyntaxTree& tree);
    std::map<std::string, std::string> getConstants() const;
    std::vector<symbols::ValueEntry> getDataHomes() const;

private:
    SemanticAnalysisVisitor analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYZER_H_
