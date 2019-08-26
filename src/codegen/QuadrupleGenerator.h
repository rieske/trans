#ifndef QUADRUPLEGENERATOR_H_
#define QUADRUPLEGENERATOR_H_

#include <vector>
#include <memory>

#include "../ast/AbstractSyntaxTree.h"
#include "../parser/ParseTree.h"
#include "../parser/SyntaxTreeVisitor.h"
#include "quadruples/Quadruple.h"

namespace codegen {

class QuadrupleGenerator: public parser::SyntaxTreeVisitor {
public:
    virtual ~QuadrupleGenerator() = default;

    std::vector<std::unique_ptr<Quadruple>> generateQuadruplesFrom(parser::SyntaxTree& syntaxTree);

private:
    void visit(ast::AbstractSyntaxTree& tree) override;
    void visit(parser::ParseTree& parseTree) override;

    std::vector<std::unique_ptr<Quadruple>> quadruples;
};

} /* namespace codegen */

#endif /* QUADRUPLEGENERATOR_H_ */
