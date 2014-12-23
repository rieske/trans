#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "../code_generator/LabelEntry.h"
#include "AbstractSyntaxTreeNode.h"
#include "Expression.h"

namespace ast {

class Expression;

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    void setLoopEntry(code_generator::LabelEntry loopEntry);
    code_generator::LabelEntry* getLoopEntry() const;
    void setLoopExit(code_generator::LabelEntry loopExit);
    code_generator::LabelEntry* getLoopExit() const;

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);

private:
    std::unique_ptr<code_generator::LabelEntry> loopEntry { nullptr };
    std::unique_ptr<code_generator::LabelEntry> loopExit { nullptr };
};

}

#endif // _LOOP_HDR_NODE_H_
