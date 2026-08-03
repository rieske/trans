#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <vector>

#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeVisitor.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "symbols/ValueEntry.h"

namespace semantic_analyzer {

class SemanticAnalyzer: public parser::SyntaxTreeVisitor {
public:
    SemanticAnalyzer() = default;
    virtual ~SemanticAnalyzer();

    void analyze(parser::SyntaxTree& syntaxTree);
    std::map<std::string, std::string> getConstants() const;
    std::vector<symbols::ValueEntry> getGlobalVariables() const;

    void printSymbolTable() const;

private:
    void visit(ast::AbstractSyntaxTree& tree) override;
    void visit(parser::ParseTree& parseTree) override;

    std::unique_ptr<SemanticAnalysisVisitor> analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SYNTAXTREEBUILDERDECORATOR_H_
