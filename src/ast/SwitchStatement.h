#ifndef SWITCHSTATEMENT_H_
#define SWITCHSTATEMENT_H_

#include <memory>
#include <vector>

#include "semantic_analyzer/LabelEntry.h"
#include "semantic_analyzer/ValueEntry.h"
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

    void setExitLabel(semantic_analyzer::LabelEntry exitLabel);
    semantic_analyzer::LabelEntry* getExitLabel() const;

    void setCaseTemp(semantic_analyzer::ValueEntry temp);
    semantic_analyzer::ValueEntry* getCaseTemp() const;

    void addCase(CaseLabel* caseLabel);
    const std::vector<CaseLabel*>& getCases() const;

    void setDefaultLabel(DefaultLabel* defaultLabel);
    DefaultLabel* getDefaultLabel() const;

    const std::unique_ptr<Expression> expression;
    const std::unique_ptr<AbstractSyntaxTreeNode> body;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> exitLabel { nullptr };
    std::unique_ptr<semantic_analyzer::ValueEntry> caseTemp { nullptr };
    std::vector<CaseLabel*> cases;
    DefaultLabel* defaultLabelNode { nullptr };
};

} // namespace ast

#endif // SWITCHSTATEMENT_H_
