#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <vector>

#include "../code_generator/quadruple.h"

class SymbolTable;

namespace semantic_analyzer {

class AbstractSyntaxTreeNode;

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer();

    void analyze(AbstractSyntaxTreeNode& syntaxTreeTop);

    std::unique_ptr<SymbolTable> getSymbolTable();
    std::vector<Quadruple> getQuadrupleCode() const;

private:
    std::unique_ptr<SymbolTable> symbolTable;
    std::vector<Quadruple> quadrupleCode;
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
