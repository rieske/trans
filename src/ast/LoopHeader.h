#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "semantic_analyzer/LabelEntry.h"
#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Expression.h"

namespace ast {

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    void setLoopEntry(semantic_analyzer::LabelEntry loopEntry);
    semantic_analyzer::LabelEntry* getLoopEntry() const;
    void setLoopExit(semantic_analyzer::LabelEntry loopExit);
    semantic_analyzer::LabelEntry* getLoopExit() const;
    void setLoopContinue(semantic_analyzer::LabelEntry loopContinue);
    semantic_analyzer::LabelEntry* getLoopContinue() const;

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

private:
    std::unique_ptr<semantic_analyzer::LabelEntry> loopEntry { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> loopExit { nullptr };
    std::unique_ptr<semantic_analyzer::LabelEntry> loopContinue { nullptr };
};

} // namespace ast

#endif // _LOOP_HDR_NODE_H_
