#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

namespace semantic_analyzer {

class AbstractSyntaxTreeNode;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer();

    void analyze(const AbstractSyntaxTreeNode& syntaxTreeTop);
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
