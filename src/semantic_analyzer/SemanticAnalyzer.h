#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include "../parser/SyntaxTreeVisitor.h"

namespace parser {
    class SyntaxTree;
}

namespace semantic_analyzer {

class SemanticAnalyzer: public parser::SyntaxTreeVisitor {
public:
    virtual ~SemanticAnalyzer() = default;

    void analyze(parser::SyntaxTree& syntaxTree);

private:
    void visit(ast::AbstractSyntaxTree& tree) override;
    void visit(parser::ParseTree& parseTree) override;
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
