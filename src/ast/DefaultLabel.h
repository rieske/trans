#ifndef DEFAULTLABEL_H_
#define DEFAULTLABEL_H_

#include <memory>

#include "symbols/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"

namespace ast {

class DefaultLabel: public AbstractSyntaxTreeNode {
public:
    DefaultLabel(TerminalSymbol defaultKeyword, std::unique_ptr<AbstractSyntaxTreeNode> statement);
    virtual ~DefaultLabel() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    void setLabel(symbols::LabelEntry label);
    symbols::LabelEntry* getLabel() const;

    const TerminalSymbol defaultKeyword;
    const std::unique_ptr<AbstractSyntaxTreeNode> statement;

};

} // namespace ast

#endif // DEFAULTLABEL_H_
