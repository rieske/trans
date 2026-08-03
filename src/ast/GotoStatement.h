#ifndef GOTOSTATEMENT_H_
#define GOTOSTATEMENT_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"

namespace ast {

class GotoStatement: public AbstractSyntaxTreeNode {
public:
    GotoStatement(TerminalSymbol gotoKeyword, TerminalSymbol labelName);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    const std::string& getLabelName() const;

    TerminalSymbol gotoKeyword;
    TerminalSymbol label;

};

} // namespace ast

#endif // GOTOSTATEMENT_H_
