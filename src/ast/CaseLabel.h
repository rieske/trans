#ifndef CASELABEL_H_
#define CASELABEL_H_

#include <memory>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

namespace ast {

class CaseLabel: public Statement {
public:
    CaseLabel(std::unique_ptr<Expression> caseExpression, std::unique_ptr<Statement> statement);
    virtual ~CaseLabel() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    NodeKind nodeKind() const override { return NodeKind::CaseLabel; }

    void setLabel(symbols::AnnotationStore& store, symbols::LabelEntry label);
    symbols::LabelEntry* getLabel(symbols::AnnotationStore& store) const;

    void setCaseValue(long value);
    long getCaseValue() const;

    const std::unique_ptr<Expression> caseExpression;
    const std::unique_ptr<Statement> statement;

private:
    long caseValue { 0 };
};

} // namespace ast

#endif // CASELABEL_H_
