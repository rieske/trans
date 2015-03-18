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
    SemanticAnalyzer() = default;
    ~SemanticAnalyzer() = default;

    void analyze(std::vector<std::unique_ptr<ast::AbstractSyntaxTreeNode> >& translationUnit);

    std::vector<code_generator::Quadruple_deprecated> getQuadrupleCode() const;

private:
    std::vector<code_generator::Quadruple_deprecated> quadrupleCode;
};

}

#endif /* SYNTAXTREEBUILDERDECORATOR_H_ */
