#include "LoopHeader.h"

#include <algorithm>

namespace semantic_analyzer {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(SymbolTable* symbolTable, std::unique_ptr<Expression> increment) :
        NonterminalNode(symbolTable, 0),
        increment { std::move(increment) } {
}

vector<Quadruple *> LoopHeader::getBPList() const {
    return backpatchList;
}

SymbolEntry *LoopHeader::getLoopLabel() const {
    return loop_label;
}

vector<Quadruple *> LoopHeader::getAfterLoop() const {
    return afterLoop;
}

void LoopHeader::setLoopEntry(SymbolEntry* loopEntry) {
    this->loopEntry = loopEntry;
}

SymbolEntry* LoopHeader::getLoopEntry() const {
    return loopEntry;
}

void LoopHeader::setLoopExit(SymbolEntry* loopExit) {
    this->loopExit = loopExit;
}

SymbolEntry* LoopHeader::getLoopExit() const {
    return loopExit;
}

}
