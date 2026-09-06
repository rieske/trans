#ifndef SWITCHSTATEMENT_H_
#define SWITCHSTATEMENT_H_

#include <memory>
#include <vector>

#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

namespace ast {

class CaseLabel;
class DefaultLabel;

class SwitchStatement: public Statement {
public:
    SwitchStatement(std::unique_ptr<Expression> expression, std::unique_ptr<Statement> body);
    virtual ~SwitchStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setExitLabel(symbols::AnnotationStore& store, symbols::LabelEntry exitLabel);
    symbols::LabelEntry* getExitLabel(symbols::AnnotationStore& store) const;

    void setCaseTemp(symbols::AnnotationStore& store, symbols::ValueEntry temp);
    symbols::ValueEntry* getCaseTemp(symbols::AnnotationStore& store) const;

    void addCase(CaseLabel* caseLabel);
    const std::vector<CaseLabel*>& getCases() const;

    void setDefaultLabel(DefaultLabel* defaultLabel);
    DefaultLabel* getDefaultLabel() const;

    const std::unique_ptr<Expression> expression;
    const std::unique_ptr<Statement> body;

private:
    std::vector<CaseLabel*> cases;
    DefaultLabel* defaultLabelNode { nullptr };
};

} // namespace ast

#endif // SWITCHSTATEMENT_H_
