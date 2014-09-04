#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "Expression.h"

#include "../code_generator/symbol_entry.h"

namespace ast {

class LoopHeader: public AbstractSyntaxTreeNode {
public:
    SymbolEntry *getLoopLabel() const;

    void setLoopEntry(SymbolEntry* loopEntry);
    SymbolEntry* getLoopEntry() const;
    void setLoopExit(SymbolEntry* loopExit);
    SymbolEntry* getLoopExit() const;

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(std::unique_ptr<Expression> increment = nullptr);

    SymbolEntry *loop_label;
private:
    SymbolEntry* loopEntry { nullptr };
    SymbolEntry* loopExit { nullptr };
};

}

#endif // _LOOP_HDR_NODE_H_
