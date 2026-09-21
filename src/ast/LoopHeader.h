#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

enum class LoopKind {
    While,
    For,
    DoWhile
};

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    virtual LoopKind loopKind() const = 0;

    void setLoopEntry(symbols::AnnotationStore& store, symbols::LabelEntry loopEntry);
    symbols::LabelEntry* getLoopEntry(symbols::AnnotationStore& store) const;
    void setLoopExit(symbols::AnnotationStore& store, symbols::LabelEntry loopExit);
    symbols::LabelEntry* getLoopExit(symbols::AnnotationStore& store) const;
    void setLoopContinue(symbols::AnnotationStore& store, symbols::LabelEntry loopContinue);
    symbols::LabelEntry* getLoopContinue(symbols::AnnotationStore& store) const;

    std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);

};

} // namespace ast

#endif // _LOOP_HDR_NODE_H_
