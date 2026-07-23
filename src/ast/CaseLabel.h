#ifndef CASELABEL_H_
#define CASELABEL_H_

#include <memory>

#include "semantic_analyzer/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class CaseLabel: public AbstractSyntaxTreeNode {
public:
    CaseLabel(std::unique_ptr<Expression> caseExpression, std::unique_ptr<AbstractSyntaxTreeNode> statement);
    virtual ~CaseLabel() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getLabel() const;

    void setCaseValue(long value);
    long getCaseValue() const;

    const std::unique_ptr<Expression> caseExpression;
    const std::unique_ptr<AbstractSyntaxTreeNode> statement;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> label { nullptr };
    long caseValue { 0 };
};

} // namespace ast

#endif // CASELABEL_H_
