#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <vector>

#include "../code_generator/quadruple.h"

namespace code_generator {
class SymbolTable;
}

namespace ast {
class AbstractSyntaxTreeNode;
}

namespace semantic_analyzer {

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer();

    void analyze(ast::AbstractSyntaxTreeNode& syntaxTreeTop);

    std::unique_ptr<code_generator::SymbolTable> getSymbolTable();
    std::vector<code_generator::Quadruple> getQuadrupleCode() const;

private:
    std::unique_ptr<code_generator::SymbolTable> symbolTable;
    std::vector<code_generator::Quadruple> quadrupleCode;
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
