#ifndef LABELEDSTATEMENT_H_
#define LABELEDSTATEMENT_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "semantic_analyzer/LabelEntry.h"

namespace ast {

// C statement label: id : statement
class LabeledStatement: public AbstractSyntaxTreeNode {
public:
    LabeledStatement(TerminalSymbol labelName, std::unique_ptr<AbstractSyntaxTreeNode> statement);
    virtual ~LabeledStatement() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(semantic_analyzer::LabelEntry label);
    semantic_analyzer::LabelEntry* getLabel() const;

    const std::string& getLabelName() const;

    TerminalSymbol name;
    const std::unique_ptr<AbstractSyntaxTreeNode> statement;

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> label { nullptr };
};

} // namespace ast

#endif // LABELEDSTATEMENT_H_
