#ifndef SEMANTICANALYZER_H_
#define SEMANTICANALYZER_H_

#include <map>
#include <vector>

#include "ast/AbstractSyntaxTree.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "semantic_analyzer/ValueEntry.h"

namespace semantic_analyzer {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(bool gnuExtensions = true);
    ~SemanticAnalyzer();

    void analyze(ast::AbstractSyntaxTree& tree);
    std::map<std::string, std::string> getConstants() const;
    std::vector<ValueEntry> getDataHomes() const;

private:
    SemanticAnalysisVisitor analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYZER_H_
