#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

namespace semantic_analyzer {

class AbstractSyntaxTree;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer();

    void analyze(const AbstractSyntaxTree& syntaxTree);
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
