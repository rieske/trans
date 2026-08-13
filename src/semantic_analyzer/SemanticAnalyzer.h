#ifndef SEMANTIC_ANALYZER_H_
#define SEMANTIC_ANALYZER_H_

#include <map>
#include <string>
#include <vector>

#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeVisitor.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "symbols/ValueEntry.h"

namespace parser {
class ParseExtensions;
}

namespace semantic_analyzer {

using symbols::ValueEntry;

class SemanticAnalyzer: public parser::SyntaxTreeVisitor {
public:
    explicit SemanticAnalyzer(const parser::ParseExtensions* extensions = nullptr);
    virtual ~SemanticAnalyzer();

    void analyze(parser::SyntaxTree& syntaxTree);
    std::map<std::string, std::string> getConstants() const;
    std::vector<ValueEntry> getDataHomes() const;

private:
    void visit(ast::AbstractSyntaxTree& tree) override;
    void visit(parser::ParseTree& parseTree) override;

    SemanticAnalysisVisitor analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SEMANTIC_ANALYZER_H_
