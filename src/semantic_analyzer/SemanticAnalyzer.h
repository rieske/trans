#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <iostream>

#include "parser/SyntaxTree.h"
#include "parser/SyntaxTreeVisitor.h"
#include "semantic_analyzer/SemanticAnalysisVisitor.h"

namespace semantic_analyzer {

class SemanticAnalyzer: public parser::SyntaxTreeVisitor {
public:
    SemanticAnalyzer(): analyzerVisitor { &std::cerr } {}
    virtual ~SemanticAnalyzer();

    void analyze(parser::SyntaxTree& syntaxTree);
    std::map<std::string, std::string> getConstants() const;

    void printSymbolTable() const;

private:
    void visit(ast::AbstractSyntaxTree& tree) override;
    void visit(parser::ParseTree& parseTree) override;

    SemanticAnalysisVisitor analyzerVisitor;
};

} // namespace semantic_analyzer

#endif // SYNTAXTREEBUILDERDECORATOR_H_
