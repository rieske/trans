#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <iostream>
#include <vector>

#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeVisitor.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"
#include "semantic_analyzer/ValueEntry.h"

namespace parser {
class ParseExtensions;
}

namespace semantic_analyzer {

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

#endif // SYNTAXTREEBUILDERDECORATOR_H_
