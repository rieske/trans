#ifndef VOID_RETURNSTATEMENT_H_
#define VOID_RETURNSTATEMENT_H_

#include "ast/Statement.h"

namespace ast {

class VoidReturnStatement: public Statement {
public:
    VoidReturnStatement() = default;
    virtual ~VoidReturnStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
};

} // namespace ast

#endif // VOID_RETURNSTATEMENT_H_
