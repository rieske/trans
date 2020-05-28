#ifndef VOID_RETURNSTATEMENT_H_
#define VOID_RETURNSTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Expression;

class VoidReturnStatement: public AbstractSyntaxTreeNode {
public:
    VoidReturnStatement() = default;
    virtual ~VoidReturnStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} /* namespace ast */

#endif /* VOID_RETURNSTATEMENT_H_ */
