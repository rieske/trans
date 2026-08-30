#ifndef SEMANTICANALYZER_H_
#define SEMANTICANALYZER_H_

#include <map>
#include <string>
#include <vector>

#include "ast/AbstractSyntaxTree.h"
#include "scanner/LexicalSession.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"

namespace diag {
class Sink;
}

namespace semantic_analyzer {

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(bool gnuExtensions = true);
    ~SemanticAnalyzer();

    bool analyze(ast::AbstractSyntaxTree& tree, const scanner::LexicalSession& session,
            diag::Sink& sink);
    std::map<std::string, std::string> getConstants() const;
    std::vector<symbols::ValueEntry> getDataHomes() const;

private:
    SemanticAnalysisVisitor analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SEMANTICANALYZER_H_
