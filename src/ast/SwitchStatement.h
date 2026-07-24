#ifndef SWITCHSTATEMENT_H_
#define SWITCHSTATEMENT_H_

#include <memory>
#include <vector>

#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class CaseLabel;
class DefaultLabel;

class SwitchStatement: public AbstractSyntaxTreeNode {
public:
    SwitchStatement(std::unique_ptr<Expression> expression, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~SwitchStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setExitLabel(symbols::LabelEntry exitLabel);
    symbols::LabelEntry* getExitLabel() const;

    void setCaseTemp(symbols::ValueEntry temp);
    symbols::ValueEntry* getCaseTemp() const;

    void addCase(CaseLabel* caseLabel);
    const std::vector<CaseLabel*>& getCases() const;

    void setDefaultLabel(DefaultLabel* defaultLabel);
    DefaultLabel* getDefaultLabel() const;

    const std::unique_ptr<Expression> expression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    // Structural (which case/default nodes belong to this switch) — not SA symbols.
    std::vector<CaseLabel*> cases;
    DefaultLabel* defaultLabelNode { nullptr };
};

} // namespace ast

#endif // SWITCHSTATEMENT_H_
