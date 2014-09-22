#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "Expression.h"

namespace code_generator {
class LabelEntry;
} /* namespace code_generator */

namespace ast {

class Expression;

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    code_generator::LabelEntry *getLoopLabel() const;

    void setLoopEntry(code_generator::LabelEntry* loopEntry);
    code_generator::LabelEntry* getLoopEntry() const;
    void setLoopExit(code_generator::LabelEntry* loopExit);
    code_generator::LabelEntry* getLoopExit() const;

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);

    code_generator::LabelEntry *loop_label { nullptr };
private:
    code_generator::LabelEntry* loopEntry { nullptr };
    code_generator::LabelEntry* loopExit { nullptr };
};

}

#endif // _LOOP_HDR_NODE_H_
