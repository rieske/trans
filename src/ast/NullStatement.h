#ifndef NULLSTATEMENT_H_
#define NULLSTATEMENT_H_

#include "Statement.h"

namespace ast {

class NullStatement: public Statement {
public:
    NullStatement() = default;
    virtual ~NullStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    NodeKind nodeKind() const override { return NodeKind::NullStatement; }
};

} // namespace ast

#endif
