#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    NodeKind nodeKind() const override { return NodeKind::LoopHeader; }

    void setLoopEntry(symbols::AnnotationStore& store, symbols::LabelEntry loopEntry);
    symbols::LabelEntry* getLoopEntry(symbols::AnnotationStore& store) const;
    void setLoopExit(symbols::AnnotationStore& store, symbols::LabelEntry loopExit);
    symbols::LabelEntry* getLoopExit(symbols::AnnotationStore& store) const;
    void setLoopContinue(symbols::AnnotationStore& store, symbols::LabelEntry loopContinue);
    symbols::LabelEntry* getLoopContinue(symbols::AnnotationStore& store) const;

    // C99 for-with-declaration scopes the header declaration over the loop body.
    virtual bool opensBlockScope() const { return false; }

    // do-while: body before condition. while/for: test before body.
    virtual bool bodyBeforeTest() const { return false; }
    // while (and for without increment): continue → entry. do-while: continue → test.
    virtual bool continueTargetsEntry() const { return !increment; }

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);

};

} // namespace ast

#endif // _LOOP_HDR_NODE_H_
