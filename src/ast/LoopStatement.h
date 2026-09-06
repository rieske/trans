#ifndef LOOPSTATEMENT_H_
#define LOOPSTATEMENT_H_

#include <memory>

#include "ast/LoopHeader.h"
#include "ast/Statement.h"

namespace ast {

class LoopStatement: public Statement {
public:
    LoopStatement(std::unique_ptr<LoopHeader> header, std::unique_ptr<Statement> body);
    virtual ~LoopStatement();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    NodeKind nodeKind() const override { return NodeKind::LoopStatement; }

    const std::unique_ptr<LoopHeader> header;
    const std::unique_ptr<Statement> body;
};

} // namespace ast

#endif // LOOPSTATEMENT_H_
