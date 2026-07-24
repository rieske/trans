#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "symbols/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    void setLoopEntry(symbols::LabelEntry loopEntry);
    symbols::LabelEntry* getLoopEntry() const;
    void setLoopContinue(symbols::LabelEntry loopContinue);
    symbols::LabelEntry* getLoopContinue() const;
    void setLoopExit(symbols::LabelEntry loopExit);
    symbols::LabelEntry* getLoopExit() const;

    // C99 for-with-declaration scope: ForLoopHeader::isDeclarationInit().

    // do-while: body before condition. while/for: test before body.
    virtual bool bodyBeforeTest() const { return false; }
    // Common test expression (while/do-while clause; for optional).
    virtual Expression* testExpression() { return nullptr; }
    virtual const Expression* testExpression() const { return nullptr; }

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);
};

} // namespace ast

#endif // _LOOP_HDR_NODE_H_
