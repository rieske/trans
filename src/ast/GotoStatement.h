#ifndef GOTOSTATEMENT_H_
#define GOTOSTATEMENT_H_

#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class GotoStatement: public AbstractSyntaxTreeNode {
public:
    GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    NodeKind nodeKind() const override { return NodeKind::GotoStatement; }

    void setTarget(symbols::AnnotationStore& store, symbols::LabelEntry target);
    symbols::LabelEntry* getTarget(symbols::AnnotationStore& store) const;

    const std::string& getLabelName() const;

    TerminalSymbol gotoKeyword;
    TerminalSymbol label;

};

} // namespace ast

#endif // GOTOSTATEMENT_H_
