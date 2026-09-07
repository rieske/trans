#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "ast/Expression.h"
#include "ast/Statement.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class IfStatement: public Statement {
public:
    IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<Statement> body,
            std::unique_ptr<Statement> elseBody = nullptr);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel);
    symbols::LabelEntry* getFalsyLabel(symbols::AnnotationStore& store) const;
    void setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel);
    symbols::LabelEntry* getExitLabel(symbols::AnnotationStore& store) const;

    const std::unique_ptr<Expression> testExpression;
    const std::unique_ptr<Statement> body;
    const std::unique_ptr<Statement> elseBody;
};

} // namespace ast

#endif // IFSTATEMENT_H_
