#ifndef SYNTAXTREEBUILDERDECORATOR_H_
#define SYNTAXTREEBUILDERDECORATOR_H_

#include <memory>
#include <vector>

#include "code_generator/quadruples/Quadruple.h"

namespace ast {
class AbstractSyntaxTreeNode;
}

namespace semantic_analyzer {

class SemanticAnalyzer {
public:
    SemanticAnalyzer() = default;
    ~SemanticAnalyzer() = default;

    void analyze(std::vector<std::unique_ptr<ast::AbstractSyntaxTreeNode> >& translationUnit);

    std::vector<std::unique_ptr<codegen::Quadruple>> getQuadrupleCode();

private:
    std::vector<std::unique_ptr<codegen::Quadruple>> quadrupleCode;
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
