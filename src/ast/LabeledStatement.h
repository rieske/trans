#ifndef LABELEDSTATEMENT_H_
#define LABELEDSTATEMENT_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "symbols/LabelEntry.h"

namespace ast {

// C statement label: id : statement
class LabeledStatement: public AbstractSyntaxTreeNode {
public:
    LabeledStatement(TerminalSymbol labelName, std::unique_ptr<AbstractSyntaxTreeNode> statement);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(symbols::LabelEntry label);
    symbols::LabelEntry* getLabel() const;

    const std::string& getLabelName() const;

    TerminalSymbol name;
    const std::unique_ptr<AbstractSyntaxTreeNode> statement;

};

} // namespace ast

#endif // LABELEDSTATEMENT_H_
