#ifndef VARIABLEDEFINITION_H_
#define VARIABLEDEFINITION_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Expression;
class VariableDeclaration;

class VariableDefinition: public AbstractSyntaxTreeNode {
public:
    VariableDefinition(std::unique_ptr<VariableDeclaration> declaration,
            std::unique_ptr<Expression> initializerExpression);
    virtual ~VariableDefinition();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const std::unique_ptr<VariableDeclaration> declaration;
    std::unique_ptr<Expression> initializerExpression;
};

} /* namespace ast */

#endif /* VARIABLEDEFINITION_H_ */
