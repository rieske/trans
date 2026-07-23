#ifndef GOTOSTATEMENT_H_
#define GOTOSTATEMENT_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "semantic_analyzer/LabelEntry.h"

namespace ast {

class GotoStatement: public AbstractSyntaxTreeNode {
public:
    GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName);
    virtual ~GotoStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setTarget(semantic_analyzer::LabelEntry target);
    semantic_analyzer::LabelEntry* getTarget() const;

    const std::string& getLabelName() const;

    TerminalSymbol gotoKeyword;
    TerminalSymbol label;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> target { nullptr };
};

} // namespace ast

#endif // GOTOSTATEMENT_H_
