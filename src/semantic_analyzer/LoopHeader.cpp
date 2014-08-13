#include "LoopHeader.h"

#include <algorithm>

#include "Expression.h"

namespace semantic_analyzer {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(std::unique_ptr<Expression> increment) :
        increment { std::move(increment) } {
}

SymbolEntry *LoopHeader::getLoopLabel() const {
    return loop_label;
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
