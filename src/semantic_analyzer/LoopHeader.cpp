#include "LoopHeader.h"

namespace semantic_analyzer {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(SymbolTable* symbolTable) :
        NonterminalNode(symbolTable, 0) {
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

const SymbolEntry* LoopHeader::getLoopEntry() const {
    return loopEntry;
}

void LoopHeader::setLoopExit(SymbolEntry* loopExit) {
    this->loopExit = loopExit;
}

const SymbolEntry* LoopHeader::getLoopExit() const {
    return loopExit;
}

}
