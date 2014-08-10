#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "Expression.h"
#include "NonterminalNode.h"

namespace semantic_analyzer {

class LoopHeader: public NonterminalNode {
public:
    vector<Quadruple *> getBPList() const;
    vector<Quadruple *> getAfterLoop() const;
    SymbolEntry *getLoopLabel() const;

    void setLoopEntry(SymbolEntry* loopEntry);
    SymbolEntry* getLoopEntry() const;
    void setLoopExit(SymbolEntry* loopExit);
    SymbolEntry* getLoopExit() const;

    static const std::string ID;

    const std::unique_ptr<Expression> increment;

protected:
    LoopHeader(SymbolTable* symbolTable, std::unique_ptr<Expression> increment = nullptr);

    vector<Quadruple *> backpatchList;
    vector<Quadruple *> afterLoop;
    SymbolEntry *loop_label;

private:
    SymbolEntry* loopEntry { nullptr };
    SymbolEntry* loopExit { nullptr };
};

}

#endif // _LOOP_HDR_NODE_H_
